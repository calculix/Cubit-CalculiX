# Cubit VTK development files

These tools prepare dependencies for the in-memory dataset converter. They do
not create a postprocessing workspace, viewport, or replacement VTK
runtime. The archived writer remains unbound.

The default source location is the sibling `../VTK-9.4.2` directory under `Git`.
The sources remain there. A prepared SDK refers to both that source tree and its
own generated header directory; keep both available and regenerate after moving
either directory. `--source`, `-VtkSource`, and CMake's `VTK_SOURCE_DIR` override
the default.

## Normal clean build

Use the normal CMake Configure and Build actions in VS Code. On Windows, the
first configuration automatically prepares missing VTK development files under
`<build-directory>/vtk-sdk`. No existing build directory, manually prepared SDK
or `CUBIT_VTK_SDK_DIR` cache setting is required. Deleting the build directory
causes these files to be regenerated on the next configuration.

CMake locates Python and the selected MSVC toolchain's librarian, then inspects
Cubit's DLLs, configures headers from the sibling VTK sources and generates the
import libraries. It passes the selected Visual Studio instance and toolset to
the nested header configuration. This includes compiler probes but never builds
replacement VTK runtime libraries. The initial configuration takes longer.

The defaults use `../VTK-9.4.2` and `settings.example.json`. Override
`VTK_SOURCE_DIR`, `CUBIT_ROOT` or `CUBIT_VTK_SETTINGS_FILE` when needed. An explicit
`CUBIT_VTK_SDK_DIR` can still select a prepared development directory. Existing
adapters retain their version, installation and DLL fingerprint checks.

## Optional inspection

From the repository root, with normal Python (no compiler required):

```powershell
python -B tools/vtk/inspect_cubit_vtk.py `
  --cubit-root 'C:/Program Files/Coreform Cubit 2026.11' `
  --output build/vtk-inspection.json
```

This reads PE import/export tables without loading the DLLs. The report records
SHA-256 hashes, architecture, version strings, namespace evidence and exports.
It distinguishes the 11 dataset dependency DLLs from VTK dependencies referenced
by Cubit's existing SDK CMake targets. Those additional targets must resolve when
linking the existing Cubit GUI; they do not add a plugin viewport or rendering
feature. No runtime DLLs are copied.

Optional: run the same script with `--runtime-probe` using a **fresh Python 3.12
process** able to load Cubit's wrappers. This loads wrappers only from
the selected installation and records `vtkIdType` size and the SMP backend when
exposed by the wrappers. Static inspection cannot establish all header settings.
The probe cannot certify C++ binary compatibility.

## Optional manual preparation

`settings.example.json` contains explicit **candidate settings**, not verified
Coreform build flags. Copy it to a local settings file if changes are needed.
Namespace settings must agree with the exported C++ symbols. If the runtime
probe was included, the selected ID width must agree with it. Future bool/const
options, source patches and compiler details remain unverified until the C++
checks in stage 4. The script reports this uncertainty instead of claiming a
compatible SDK.

```powershell
./tools/vtk/prepare_cubit_vtk_sdk.ps1 `
  -InspectionReport build/vtk-inspection.json `
  -SettingsFile tools/vtk/settings.example.json `
  -OutputDirectory build/vtk-sdk
```

The default writes `preparation-plan.json` only. It does not run CMake, compiler
probes, or a librarian. The plan selects `CommonCore`/`CommonDataModel`,
`IOXML`/`IOParallelXML` and their
source dependency closure; it disables tests, examples, remote modules, logging,
wrappers and unrelated module groups. Bundled VTK third-party sources are used.
Boost remains the converter's worker implementation; no TBB or OpenMP runtime is
introduced by the helper.

### Run the manual preparation

Run the same command with `-Prepare` in an x64 Visual Studio 2022 developer shell.
This operation **does configure a C/C++ project and run compiler probes**. It
does not build VTK libraries. It uses CMake to generate headers and `lib.exe` to
generate x64 import libraries from the actual Cubit DLL exports, including DATA
exports. Named forwarded exports retain their public names.

Generated output:

- `header-project/` and `header-build/`: configuration and generated headers.
- `lib/`: export definitions and import libraries, including dependencies needed
  by Cubit's existing SDK targets.
- `inspection.json` and `preparation-plan.json`: provenance and selected settings.
- `cubit-vtk-sdk.cmake`: adapter, published only after successful preparation.

Once preparation starts, a failure removes the previous adapter so a partial result cannot be
consumed accidentally. Preparation refuses overlapping source, output and host
directories. It never modifies the VTK sources or Cubit installation.

## Select an externally prepared directory

Only when using a development directory outside the default build tree, pass
these cache values to the plugin configuration command:

```text
-DCUBIT_ROOT=C:/Program Files/Coreform Cubit 2026.11
-DCUBIT_VTK_SDK_DIR=<absolute path to build/vtk-sdk>
```

`VTK_SOURCE_DIR` defaults to the sibling source tree. The adapter checks exact
version 9.4.2, the selected host/source roots, DLL hashes, generated headers and
import-library availability. It supplies targets before loading Cubit's SDK
exports and never falls back to another installed VTK. Use **Release**, matching
Cubit; Debug compatibility is not provided. CMake runs preparation automatically
when the selected development directory has no adapter. The prepared SDK remains labelled `unvalidated` until
stage-4 compilation and runtime tests have passed.

The preparation helper is Windows-specific. Linux still requires an explicit
host-compatible VTK 9.4.2 SDK through `VTK_DIR`; no Linux SDK is generated here.

## Deployment boundary

| Dependency | Current treatment |
| --- | --- |
| VTK | Use Cubit's DLLs; no VTK runtime copied by this plugin |
| Boost.Asio, CGAL, Eigen | Headers/static compilation; no new runtime DLLs |
| HDF5 C/C++ | Keep current packaging; HL is no longer requested |
| Qt Charts | Keep current packaging; align/validate against Cubit's Qt separately |
| CalculiX MKL/OpenMP | Keep existing libraries in the solver directory |

The in-memory converter requires no VTK XML or Qt integration module. The
separate exporter uses IOXML and IOParallelXML (local writes do not need MPI).
No postprocessing workspace is present. The standalone checks under `tests/vtk`
exercise production conversion and export against Cubit's runtime without a
viewport; see `docs/developer/vtk-conversion.rst` for the test commands.
