# CalculiX 2.23 for Windows x64

Runtime files: `ccx_2.23.exe` and `libiomp5md.dll`.
MKL PARDISO, ARPACK and the Intel Fortran runtime are linked statically.
PARDISO uses Intel OpenMP for parallel execution.

Intel's OpenMP DLL requires the Microsoft Visual C++ 2015-2022 x64
Redistributable (`VCRUNTIME140.dll`). Install that runtime on target machines.
Keep the notices in `licenses` with redistributed binaries.

Sources: https://www.dhondt.de/ccx_2.23.src.tar.bz2 (CCX 2.23),
ARPACK-ng 3.9.1, Intel IFX 2025.3.3, MKL 2025.3.1 and OpenMP 2025.3.3.
The reproducible build setup is in the sibling `CalculiX` checkout under
the Git directory (`build-mkl.ps1` and `README-build.md`).
The `solidsections.f` input parser uses `/Od` to avoid IFX hoisting a read
from an unused null cyclic-symmetry array; numerical routines use `/O2`.
The upstream banner contains a hardcoded October 2025 timestamp.

Validated with HEX8/HEX20 elastic patch tests, steady heat conduction,
ARPACK eigenmodes and nonlinear contact, using two PARDISO threads.
These smoke tests do not cover every CalculiX procedure.

CMake stages these files in the active configuration's `Solver` directory.
The default option is `Solver/ccx_2.23.exe`; existing saved solver paths
are retained and can be changed in Options.
