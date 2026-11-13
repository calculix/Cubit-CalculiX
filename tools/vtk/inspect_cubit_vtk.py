"""Read Cubit's Windows VTK binaries without loading or modifying them.

Optional --runtime-probe must run with Cubit's Python and only loads wrappers
from the specified installation. Static evidence is not an ABI certification.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import sys


class PE:
    """Minimal PE reader for named exports and normal/delay DLL imports."""

    def __init__(self, path):
        self.path = Path(path).resolve()
        self.data = self.path.read_bytes()
        if self.data[:2] != b"MZ":
            raise ValueError(f"Not a PE file: {self.path}")
        pe = self.u32(60)
        if self.data[pe:pe + 4] != b"PE\0\0":
            raise ValueError(f"Invalid PE signature: {self.path}")
        self.machine = self.u16(pe + 4)
        optional = pe + 24
        magic = self.u16(optional)
        if magic not in (0x10B, 0x20B):
            raise ValueError("Unsupported PE optional header")
        self.image_base = struct.unpack_from("<Q" if magic == 0x20B else "<I", self.data, optional + (24 if magic == 0x20B else 28))[0]
        self.directory = optional + (112 if magic == 0x20B else 96)
        self.sections = []
        sections = optional + self.u16(pe + 20)
        for index in range(self.u16(pe + 6)):
            offset = sections + 40 * index
            self.sections.append((self.u32(offset + 12), max(self.u32(offset + 8), self.u32(offset + 16)), self.u32(offset + 20)))

    def u16(self, offset):
        return struct.unpack_from("<H", self.data, offset)[0]

    def u32(self, offset):
        return struct.unpack_from("<I", self.data, offset)[0]

    def offset(self, rva):
        for address, size, raw in self.sections:
            if address <= rva < address + size:
                return raw + rva - address
        if rva < min(raw for _, _, raw in self.sections):
            return rva
        raise ValueError(f"Unmapped RVA {rva:x} in {self.path}")

    def string(self, rva):
        offset = self.offset(rva)
        return self.data[offset:self.data.index(b"\0", offset)].decode("ascii")

    def imports(self):
        result = []
        rva = self.u32(self.directory + 8)
        if rva:
            offset = self.offset(rva)
            while any(self.data[offset:offset + 20]):
                result.append({"name": self.string(self.u32(offset + 12)), "delay": False})
                offset += 20
        rva = self.u32(self.directory + 13 * 8)
        if rva:
            offset = self.offset(rva)
            while any(self.data[offset:offset + 32]):
                address = self.u32(offset + 4)
                if not self.u32(offset) & 1:
                    address -= self.image_base
                result.append({"name": self.string(address), "delay": True})
                offset += 32
        return result

    def exports(self):
        rva = self.u32(self.directory)
        if not rva:
            return []
        table = self.offset(rva)
        functions = self.offset(self.u32(table + 28))
        names = self.offset(self.u32(table + 32))
        ordinals = self.offset(self.u32(table + 36))
        result = []
        for index in range(self.u32(table + 24)):
            name = self.string(self.u32(names + 4 * index))
            address = self.u32(functions + 4 * self.u16(ordinals + 2 * index))
            # Data imports need DATA in the generated .def. Forwarders retain
            # their public name; the loader resolves them through Cubit's DLL.
            data = False
            for section_index, (start, size, _) in enumerate(self.sections):
                if start <= address < start + size:
                    pe = self.u32(60)
                    section = pe + 24 + self.u16(pe + 20) + section_index * 40
                    data = not bool(self.u32(section + 36) & 0x20000000)
                    break
            if rva <= address < rva + self.u32(self.directory + 4):
                data = name.startswith("?") and "@@3" in name
            result.append({"name": name, "data": data})
        return result


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def inspect(cubit_root):
    root = Path(cubit_root).resolve()
    binary = root / "bin"
    inventory = {path.name.lower(): path for path in binary.glob("*.dll")}
    dataset_roots = ["vtkcommoncore-9.4.dll", "vtkcommondatamodel-9.4.dll"]
    # Existing Cubit GUI targets also reference VTK targets. Represent their
    # binaries for linking; this does not create a viewport or configure those
    # source modules. Keep their provenance separate from dataset requirements.
    host_targets = sorted({name for path in (root / "cmake").glob("*.cmake")
                           for name in re.findall(r"VTK::([A-Za-z0-9_]+)", path.read_text(encoding="utf-8"))})
    pending = list(dataset_roots)
    pending.append("vtkioparallelxml-9.4.dll")
    # Cubit packages QVTK's Qt widget implementation inside showviz_qt.dll.
    pending.append("showviz_qt.dll")
    pending.extend(["vtkchartscore-9.4.dll", "vtkviewscontext2d-9.4.dll", "vtkrenderingcontextopengl2-9.4.dll"])
    pending.extend("vtk" + name.lower() + "-9.4.dll" for name in host_targets)
    libraries = []
    visited = set()
    while pending:
        name = pending.pop(0)
        if name in visited:
            continue
        visited.add(name)
        if name not in inventory:
            raise ValueError(f"Required VTK dependency missing: {binary / name}")
        path = inventory[name]
        pe = PE(path)
        imports = pe.imports()
        exports = pe.exports()
        versions = sorted(set(match.decode() for match in re.findall(rb"vtk version ([0-9]+\.[0-9]+\.[0-9]+)", pe.data)))
        libraries.append({"name": path.name, "path": str(path), "sha256": digest(path), "machine": pe.machine,
                          "versions": versions, "imports": imports, "exports": exports})
        pending.extend(item["name"].lower() for item in imports if item["name"].lower().startswith("vtk"))
    core = next(item for item in libraries if item["name"].lower() == "vtkcommoncore-9.4.dll")
    namespaces = []
    for item in core["exports"]:
        match = re.match(r"^\?New@vtkDoubleArray@(?:(\w+)@)?@", item["name"])
        if match:
            namespaces.append(match.group(1) or "")
    by_name = {item["name"].lower(): item for item in libraries}
    dataset_closure = set()
    pending = list(dataset_roots)
    while pending:
        name = pending.pop()
        if name in dataset_closure:
            continue
        dataset_closure.add(name)
        pending.extend(item["name"].lower() for item in by_name[name]["imports"] if item["name"].lower().startswith("vtk"))
    return {"schema": 2, "cubit_root": str(root), "scope": "Dataset/rendering dependencies plus Cubit's bundled QVTK implementation",
            "dataset_libraries": sorted(by_name[name]["name"] for name in dataset_closure), "host_sdk_targets": host_targets,
            "compatibility": "unvalidated", "libraries": libraries,
            "configuration_evidence": {"abi_namespaces_from_exports": sorted(set(namespaces)),
                                       "id_type_bytes": "unknown without --runtime-probe",
                                       "future_bool": "unknown", "future_const": "unknown",
                                       "crt_imports": sorted({item["name"] for library in libraries for item in library["imports"]
                                                              if item["name"].upper().startswith(("MSVCP", "VCRUNTIME"))})},
            "unresolved": ["Compiler/CRT build flags", "VTK header configuration (including future bool/const options)",
                           "Source patches and exact build provenance", "C++ binary compatibility requires step-4 tests"]}


def runtime_probe(root):
    binary = Path(root).resolve() / "bin"
    package = binary / "site-packages"
    if "vtkmodules" in sys.modules:
        raise ValueError("Run the runtime probe in a fresh Cubit Python process")
    sys.path.insert(0, str(package))
    dll_directory = os.add_dll_directory(str(binary))
    try:
        import vtkmodules
        if Path(vtkmodules.__file__).resolve().parent != (package / "vtkmodules").resolve():
            raise ValueError("The imported VTK wrappers do not belong to the selected Cubit")
        from vtkmodules import vtkCommonCore
        vtkIdTypeArray, vtkVersion = vtkCommonCore.vtkIdTypeArray, vtkCommonCore.vtkVersion
        smp = getattr(vtkCommonCore, "vtkSMPTools", None)
        return {"version": vtkVersion.GetVTKVersion(), "id_type_bytes": vtkIdTypeArray().GetDataTypeSize(),
                "smp_backend": smp.GetBackend() if smp is not None else "not exposed by wrappers",
                "wrapper_path": str(Path(vtkmodules.__file__).resolve()),
                "note": "Wrapper/runtime observation; not a C++ ABI test"}
    finally:
        dll_directory.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cubit-root", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--runtime-probe", action="store_true")
    args = parser.parse_args()
    report = inspect(args.cubit_root)
    if args.runtime_probe:
        report["runtime"] = runtime_probe(args.cubit_root)
        report["configuration_evidence"]["id_type_bytes"] = report["runtime"]["id_type_bytes"]
        report["configuration_evidence"]["smp_backend"] = report["runtime"]["smp_backend"]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"Recorded {len(report['dataset_libraries'])} dataset DLLs and {len(report['libraries'])} total host-linked VTK DLLs in {args.output}; compatibility remains unvalidated.")


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, ImportError, struct.error) as error:
        print(f"VTK inspection failed: {error}", file=sys.stderr)
        sys.exit(1)
