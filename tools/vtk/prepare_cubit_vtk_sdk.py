"""Prepare headers and import libraries; never build or replace VTK DLLs.

The default only writes a preparation plan. --execute explicitly runs CMake
configuration (which includes compiler probes) and Microsoft's librarian.
The resulting source/build-dependent SDK remains ABI-unvalidated.
"""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

from inspect_cubit_vtk import digest, inspect


SETTINGS = {
    "VTK_USE_64BIT_IDS": bool,
    "VTK_USE_FUTURE_BOOL": bool,
    "VTK_USE_FUTURE_CONST": bool,
    "VTK_ABI_NAMESPACE_NAME": str,
    "VTK_SMP_IMPLEMENTATION_TYPE": str,
}


def bracket(value):
    value = str(value).replace("\\", "/")
    equals = ""
    while "]" + equals + "]" in value:
        equals += "="
    return "[" + equals + "[" + value + "]" + equals + "]"


def module_info(source):
    result = {}
    for path in source.rglob("vtk.module"):
        fields = {}
        key = None
        for line in path.read_text(encoding="utf-8").splitlines():
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            if line == line.lstrip():
                key = line.strip()
                fields[key] = []
            elif key:
                fields[key].extend(line.split())
        if fields.get("NAME"):
            result[fields["NAME"][0]] = fields
    # KWSys names its library in Utilities/KWSys/CMakeLists.txt, not vtk.module.
    if "VTK::vtksys" in result:
        result["VTK::vtksys"]["LIBRARY_NAME"] = ["vtksys"]
    return result


def closure(modules):
    pending = ["VTK::CommonCore", "VTK::CommonDataModel", "VTK::IOParallelXML", "VTK::GUISupportQt"]
    result = set()
    while pending:
        name = pending.pop()
        if name in result:
            continue
        if name not in modules:
            raise ValueError(f"Missing source module {name}")
        result.add(name)
        for key in ("DEPENDS", "PRIVATE_DEPENDS"):
            pending.extend(modules[name].get(key, []))
    return sorted(result)


def validate(source, report, settings):
    version = (source / "CMake/vtkVersion.cmake").read_text(encoding="utf-8")
    numbers = [re.search(r"set\(VTK_" + part + r"_VERSION\s+(\d+)\)", version).group(1) for part in ("MAJOR", "MINOR", "BUILD")]
    if numbers != ["9", "4", "2"]:
        raise ValueError("The source tree must be VTK 9.4.2")
    if set(settings) != set(SETTINGS):
        raise ValueError(f"Settings must contain exactly: {', '.join(SETTINGS)}")
    for name, expected in SETTINGS.items():
        if type(settings[name]) is not expected:
            raise ValueError(f"Incorrect type for {name}")
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*|", settings["VTK_ABI_NAMESPACE_NAME"]):
        raise ValueError("Invalid ABI namespace")
    if settings["VTK_SMP_IMPLEMENTATION_TYPE"] not in ("Sequential", "STDThread"):
        raise ValueError("This minimal SDK supports Sequential or STDThread; no extra SMP runtime is introduced")
    for library in report["libraries"]:
        if library["machine"] != 0x8664:
            raise ValueError("Only Windows x64 DLLs are supported by this helper")
    core = next(item for item in report["libraries"] if item["name"].lower() == "vtkcommoncore-9.4.dll")
    if core["versions"] != ["9.4.2"]:
        raise ValueError("Cubit CommonCore does not identify itself as VTK 9.4.2")
    qt = next((item for item in report["libraries"] if item["name"].lower() == "showviz_qt.dll"), None)
    if qt is None or not any("?setRenderWindow@QVTKOpenGLNativeWidget@@" in item["name"] for item in qt["exports"]):
        raise ValueError("Cubit showviz_qt.dll does not export the expected QVTK widget API")
    # MSVC names encode namespace components. Check a real public C++ symbol,
    # rather than inferring namespace configuration from version text.
    namespace = settings["VTK_ABI_NAMESPACE_NAME"]
    symbol = "?New@vtkDoubleArray@" + (namespace + "@" if namespace else "") + "@"
    if not any(item["name"].startswith(symbol) for item in core["exports"]):
        raise ValueError("Selected ABI namespace does not match vtkDoubleArray::New exports")
    observed = report.get("runtime", {}).get("id_type_bytes")
    if observed is not None and observed != (8 if settings["VTK_USE_64BIT_IDS"] else 4):
        raise ValueError("Selected vtkIdType width disagrees with the runtime probe")


def wrapper(source, selected, modules):
    lines = ["cmake_minimum_required(VERSION 3.20)", "project(CubitVtkHeaders LANGUAGES C CXX)",
             "set(BUILD_SHARED_LIBS ON CACHE BOOL \"\" FORCE)", "set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)",
             "set(VTK_BUILD_TESTING OFF CACHE STRING \"\" FORCE)", "set(VTK_BUILD_EXAMPLES OFF CACHE BOOL \"\" FORCE)",
             "set(VTK_BUILD_ALL_MODULES OFF CACHE BOOL \"\" FORCE)", "set(VTK_ENABLE_REMOTE_MODULES OFF CACHE BOOL \"\" FORCE)",
             "set(VTK_FORBID_DOWNLOADS ON CACHE BOOL \"\" FORCE)", "set(VTK_WRAP_PYTHON OFF CACHE BOOL \"\" FORCE)",
             "set(VTK_WRAP_JAVA OFF CACHE BOOL \"\" FORCE)", "set(VTK_USE_EXTERNAL OFF CACHE BOOL \"\" FORCE)",
             "set(VTK_ENABLE_LOGGING OFF CACHE BOOL \"\" FORCE)", "set(VTK_MODULE_ENABLE_VTK_loguru NO CACHE STRING \"\" FORCE)",
             "set(VTK_MODULE_ENABLE_VTK_CommonCore YES CACHE STRING \"\" FORCE)",
             "set(VTK_MODULE_ENABLE_VTK_CommonDataModel YES CACHE STRING \"\" FORCE)",
             "set(VTK_MODULE_ENABLE_VTK_IOXML YES CACHE STRING \"\" FORCE)"]
    lines.append('set(VTK_MODULE_ENABLE_VTK_IOParallelXML YES CACHE STRING "" FORCE)')
    lines.append('set(VTK_QT_VERSION 6 CACHE STRING "" FORCE)')
    lines.append('set(VTK_MODULE_ENABLE_VTK_GUISupportQt YES CACHE STRING "" FORCE)')
    for group in sorted({group for fields in modules.values() for group in fields.get("GROUPS", [])}):
        lines.append(f'set(VTK_GROUP_ENABLE_{group} DONT_WANT CACHE STRING "" FORCE)')
    lines.extend(["add_subdirectory(" + bracket(source) + " vtk)", "set(_includes \"set(CUBIT_VTK_GENERATED_INCLUDE_DIRECTORIES\\n\")"])
    for name in selected:
        lines.extend([f"if(NOT TARGET {name})", f"  message(FATAL_ERROR \"Missing configured module {name}\")", "endif()",
                      f'string(APPEND _includes \"\\\"$<TARGET_GENEX_EVAL:{name},$<TARGET_PROPERTY:{name},INTERFACE_INCLUDE_DIRECTORIES>>\\\"\\n\")'])
    lines.append('string(APPEND _includes \")\\n\")')
    for prop in ("COMPILE_DEFINITIONS", "COMPILE_OPTIONS", "COMPILE_FEATURES"):
        lines.append(f'string(APPEND _includes "set(CUBIT_VTK_GENERATED_{prop}\\n")')
        for name in selected:
            lines.append(f'string(APPEND _includes "\\\"$<TARGET_GENEX_EVAL:{name},$<TARGET_PROPERTY:{name},INTERFACE_{prop}>>\\\"\\n")')
        lines.append('string(APPEND _includes \")\\n\")')
    lines.extend(['get_target_property(_core_target VTK::CommonCore ALIASED_TARGET)',
                  'if(NOT _core_target)', '  set(_core_target VTK::CommonCore)', 'endif()',
                  # Qt usage requirements differ across configurations and C/CXX.
                  # Both evaluation axes need distinct outputs.
                  'file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/sdk-includes-$<CONFIG>-$<COMPILE_LANGUAGE>.cmake" CONTENT "${_includes}" TARGET ${_core_target})'])
    return "\n".join(lines) + "\n"


def write_sdk(output, source, report, modules):
    # Publish Release C++ usage requirements, matching this C++ SDK interface.
    # Neither C metadata nor an older unqualified file is a valid fallback.
    include_file = output / "header-build/sdk-includes-Release-CXX.cmake"
    if not include_file.is_file():
        raise ValueError("CMake did not generate the Release CXX SDK include metadata")
    lines = ["# Generated development adapter; runtime DLLs remain owned by Cubit.",
             'set(CUBIT_VTK_SDK_VERSION "9.4.2")', 'set(CUBIT_VTK_SDK_COMPATIBILITY "unvalidated")',
             'set(CUBIT_VTK_SDK_FEATURES "results;viewport-qt")',
             "set(CUBIT_VTK_SDK_HOST_ROOT " + bracket(report["cubit_root"]) + ")",
             "set(CUBIT_VTK_SDK_SOURCE_ROOT " + bracket(source) + ")",
             "include(" + bracket(include_file) + ")", "list(REMOVE_DUPLICATES CUBIT_VTK_GENERATED_INCLUDE_DIRECTORIES)"]
    required_headers = ["vtkCommonCore/vtkOptions.h", "vtkCommonCore/vtkABINamespace.h",
                        "vtkCommonCore/vtkVersionMacros.h", "vtkCommonCore/vtkCommonCoreModule.h",
                        "vtkCommonCore/vtkArrayDispatchArrayList.h", "vtkCommonCore/vtkTypeListMacros.h",
                        "vtkCommonDataModel/vtkCommonDataModelModule.h",
                        "vtkRenderingCore/vtkRenderingCoreModule.h",
                        "vtkRenderingOpenGL2/vtkRenderingOpenGL2Module.h",
                        "vtkGUISupportQt/vtkGUISupportQtModule.h"]
    for relative in required_headers:
        header = output / "header-build/vtk" / relative
        if not header.is_file():
            raise ValueError(f"Missing configured header: {header}")
        lines.extend(["file(SHA256 " + bracket(header) + " _cubit_vtk_header_hash)",
                      'if(NOT _cubit_vtk_header_hash STREQUAL "' + digest(header) + '")',
                      '  message(FATAL_ERROR "Generated VTK configuration changed; regenerate SDK.")', "endif()"])
    target_for_dll = {}
    for name, fields in modules.items():
        library = fields.get("LIBRARY_NAME", [])
        if library:
            target_for_dll[(library[0] + "-9.4.dll").lower()] = name
    target_for_dll["showviz_qt.dll"] = "VTK::GUISupportQt"
    for item in report["libraries"]:
        name = item["name"]
        target = target_for_dll.get(name.lower())
        if target is None:
            raise ValueError(f"DLL {name} has no corresponding public source module")
        definitions = output / "lib" / (Path(name).stem + ".def")
        definitions.write_text('LIBRARY "' + name + '"\nEXPORTS\n' + "".join(
            "  " + entry["name"] + (" DATA" if entry["data"] else "") + "\n" for entry in item["exports"]), encoding="utf-8")
        if not item["exports"]:
            raise ValueError(f"DLL has no named exports: {name}")
        library_path = definitions.with_suffix(".lib")
        # Hash checks happen on every plugin configuration, before targets exist.
        lines.extend(["file(SHA256 " + bracket(item["path"]) + " _cubit_vtk_hash)",
                      'if(NOT _cubit_vtk_hash STREQUAL "' + item["sha256"] + '")',
                      '  message(FATAL_ERROR "Cubit VTK DLL changed; regenerate SDK: ' + name + '")', "endif()",
                      "if(NOT EXISTS " + bracket(library_path) + ")",
                      '  message(FATAL_ERROR "Missing VTK import library: ' + name + '")', "endif()",
                      f"add_library({target} SHARED IMPORTED GLOBAL)", f"set_target_properties({target} PROPERTIES",
                      "  IMPORTED_LOCATION " + bracket(item["path"]), "  IMPORTED_IMPLIB " + bracket(library_path),
                      '  INTERFACE_INCLUDE_DIRECTORIES "${CUBIT_VTK_GENERATED_INCLUDE_DIRECTORIES}"',
                      '  INTERFACE_COMPILE_DEFINITIONS "${CUBIT_VTK_GENERATED_COMPILE_DEFINITIONS}"',
                      '  INTERFACE_COMPILE_OPTIONS "${CUBIT_VTK_GENERATED_COMPILE_OPTIONS}"',
                      '  INTERFACE_COMPILE_FEATURES "${CUBIT_VTK_GENERATED_COMPILE_FEATURES}"', ")"])
    for item in report["libraries"]:
        dependencies = [target_for_dll[entry["name"].lower()] for entry in item["imports"] if entry["name"].lower() in target_for_dll]
        if item["name"].lower() == "showviz_qt.dll":
            dependencies.extend(["Qt6::Widgets", "Qt6::OpenGLWidgets"])
        if dependencies:
            lines.append("set_property(TARGET " + target_for_dll[item["name"].lower()] + " PROPERTY INTERFACE_LINK_LIBRARIES " + bracket(";".join(dependencies)) + ")")
    # Publish only after all librarian calls succeed (performed by the caller).
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[3] / "VTK-9.4.2")
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--settings", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--lib-exe", default="lib.exe")
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--generator-instance", default="")
    parser.add_argument("--toolset", default="")
    parser.add_argument("--qt-dir", default="", help="Qt6_DIR used by the selected Cubit plugin build")
    parser.add_argument("--execute", action="store_true")
    args = parser.parse_args()
    source, output = args.source.resolve(), args.output.resolve()
    if output == source or source in output.parents or output in source.parents:
        raise ValueError("The SDK output must be separate from the source tree")
    report = json.loads(args.report.read_text(encoding="utf-8-sig"))
    settings = json.loads(args.settings.read_text(encoding="utf-8-sig"))
    validate(source, report, settings)
    current = inspect(report["cubit_root"])
    expected = {item["name"]: item["sha256"] for item in report["libraries"]}
    if expected != {item["name"]: item["sha256"] for item in current["libraries"]}:
        raise ValueError("Inspection report is stale; inspect the selected Cubit again")
    host = Path(report["cubit_root"]).resolve()
    if output == host or host in output.parents or output in host.parents:
        raise ValueError("SDK output must not overlap the Cubit installation")
    modules = module_info(source)
    selected = closure(modules)
    known_dlls = {(fields["LIBRARY_NAME"][0] + "-9.4.dll").lower() for fields in modules.values() if fields.get("LIBRARY_NAME")}
    known_dlls.add("showviz_qt.dll")
    missing = sorted(name for name in expected if name.lower() not in known_dlls)
    if missing:
        raise ValueError(f"DLLs have no corresponding public source module: {missing}")
    command = [args.cmake, "-S", str(output / "header-project"), "-B", str(output / "header-build"),
               "-G", "Visual Studio 17 2022", "-A", "x64"]
    if args.generator_instance:
        command.append("-DCMAKE_GENERATOR_INSTANCE=" + args.generator_instance)
    if args.toolset:
        command.extend(["-T", args.toolset])
    if args.qt_dir:
        command.append("-DQt6_DIR=" + args.qt_dir)
    for name, value in settings.items():
        command.append("-D" + name + "=" + (("ON" if value else "OFF") if isinstance(value, bool) else value))
    plan = {"schema": 1, "source": str(source), "output": str(output), "settings": settings,
            "configuration_command": command, "modules": selected, "dlls": list(expected),
            "compatibility": "unvalidated", "actions": ["Configure headers (compiler probes)", "Generate .def and .lib from Cubit exports", "Publish CMake adapter"],
            "excluded_actions": ["Build VTK libraries", "Copy runtime DLLs", "Change Cubit installation"]}
    output.mkdir(parents=True, exist_ok=True)
    (output / "preparation-plan.json").write_text(json.dumps(plan, indent=2) + "\n", encoding="utf-8")
    if not args.execute:
        print(f"Plan written to {output / 'preparation-plan.json'}; no CMake or librarian was run.")
        return
    manifest = output / "cubit-vtk-sdk.cmake"
    # A failed re-preparation must not leave an old adapter usable.
    if manifest.exists():
        manifest.unlink()
    (output / "header-project").mkdir(exist_ok=True)
    (output / "header-project/CMakeLists.txt").write_text(wrapper(source, selected, modules), encoding="utf-8")
    subprocess.run(command, check=True)
    (output / "lib").mkdir(exist_ok=True)
    adapter = write_sdk(output, source, report, modules)
    for definitions in sorted((output / "lib").glob("*.def")):
        subprocess.run([args.lib_exe, "/nologo", "/machine:x64", "/def:" + str(definitions), "/out:" + str(definitions.with_suffix(".lib"))], check=True)
    (output / "inspection.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    manifest.write_text(adapter, encoding="utf-8")
    print(f"Prepared {manifest}; C++ binary compatibility remains unvalidated.")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, AttributeError, subprocess.CalledProcessError) as error:
        print(f"VTK SDK preparation failed: {error}", file=sys.stderr)
        sys.exit(1)
