Project utilities, Python bindings and build layout
====================================================

Utility scope
--------------

This chapter covers project-owned adapters in ``src/CalculiX/Utility`` and
``src/Framework/Utility``. It does not document external Eigen, CGAL, HDF5,
OpenPisco or GIF-library internals. The boundary is ownership/responsibility,
not merely whether a filename contains the word Utility.

.. list-table:: Project utility contracts
   :header-rows: 1
   :widths: 28 72

   * - Utility
     - Responsibility and limits
   * - ``ElementNodeOrdering``
     - CalculiX-to-Cubit/Exodus source-position permutations. It maps local positions, not global IDs; callers must supply a valid position/type.
   * - ``ElementInterpolation``
     - Supported element node counts, family matching, reference shape values/gradients and mapped Jacobian evaluation. A positive sampled determinant is not a proof of global curved-element validity.
   * - ``ElementIntegration``
     - Solver-compatible quadrature/physical weights using matching node order. Geometric weights and tensor shear conventions remain separate concerns.
   * - ``TopologyHdf5`` / ``TopologyIterationIds``
     - Structured owned-record serialization and one-based key checks. No solver/controller/mesh snapshot is serialized by these helpers.
   * - ``PersistenceTable``
     - Reusable table persistence support used by domain readers/writers; inspect each caller's row validation and commit boundary.
   * - ``CubitWrapInput`` / ``CGALTool``
     - Collect/prepare current input and write a CGAL alpha wrap as STL. Alpha/offset, overwrite and import are explicit workflow choices; no CGAL implementation details are reproduced here.
   * - ``GifAnimation``
     - Encode an ordered list of frame files with cancellation/progress values and error text. It does not own native Cubit capture or GUI logging.
   * - ``PlotChart`` / ``ScientificNumber`` / ``StopWatch``
     - Project plotting, numeric presentation and elapsed-time support. Presentation rounding is not solver input precision or a numerical tolerance.
   * - ``ThreadPool``
     - Legacy queue/worker support. Its current busy/stop/destructor behavior must be read literally; lifecycle hazards are recorded in :doc:`review-findings`.

Source files and declaration names are listed in :doc:`code-catalog/index`.
Detailed interpolation/volume/derivative mathematics is in
:doc:`topology-level-set`; solver integration and tensor conventions are in
:doc:`topology-export`.

Python boundary
----------------

``CalculiXPythonInterface`` is wrapped by SWIG into ``calculix_pythoninterface``.
The public header and ``.i`` file define the exported API and container typemaps.
The source forwards operations to existing domain interfaces. Construction of
the Python wrapper does not create a second CalculiX model.

The current container declarations include nested vectors of integers, doubles
and strings. Preserve and document each row layout. Use typed numeric values
where identity/precision matters and keyword/value rows for settings queries.
An empty output can signal query failure with a Cubit error; check the particular
method contract rather than interpreting it as a valid empty model automatically.

``get_topology_settings(id)`` returns the current 23 command-setting keys,
including dormant settings of the other method. It uses locale-independent
round-trip precision, so a Python dictionary can compare complete settings after
a command or save/load. It does not expose topology geometry integration,
nodal evaluation or proposal/acceptance helpers. Consequently, a Python journal
cannot currently verify those production calculations by calling them directly.
This is an explicit blocker, not permission to test duplicate Python equations
and claim a native pass.

Source: :download:`CalculiXPythonInterface.hpp <../../src/CalculiX/PythonInterface/CalculiXPythonInterface.hpp>`,
:download:`implementation <../../src/CalculiX/PythonInterface/CalculiXPythonInterface.cpp>`
and :download:`SWIG declarations <../../src/CalculiX/PythonInterface/CalculiXPythonInterface.i>`.

Wrapping and deployment
------------------------

Normal CMake wrapping depends on the maintained interface header and ``.i`` file.
Generated wrapper C++ and Python files are build artifacts, not maintained model
code or a place to fix a domain bug. The loaded native module and Python wrapper
must match. A stale wrapper can lack a newly declared method even when the source
checkout contains it.

Cubit/Python ABI, compiler/runtime, platform libraries and SDK version are part
of deployment compatibility. Do not replace a missing ABI-compatible module with
an arbitrary system Python package. Framework paths distinguish installation,
user options and process working directories.

Build/source organization
---------------------------

``src/CMakeLists.txt`` is the project entry point. Domain CMake files register
explicit sources with their existing target. Framework results ownership,
optional VTK serialization, storage and workspace code have different dependency
roles. CalculiX owns its domain/command/GUI implementation. Bundled binary
libraries, solver executables and external source distributions are dependencies,
not additional project modules to document as application code.

Windows and Linux paths/process behavior differ. SDK/Qt/VTK/Eigen/HDF5/CGAL and
SWIG configuration must match the selected target environment. Current defaults
in a CMake cache declaration are not proof that another installed version will
work. See the source catalogue for the maintained CMake files and
:doc:`../README` for documentation generation/staging, which is separate from
plugin compilation.

Testing and documentation-only work
------------------------------------

The standalone VTK test project under ``tests/vtk`` exercises production result
conversion. Topology configuration checks live under the maintained verification
example and are driven by the unified journal. Neither a test source file nor a
generated API page is evidence of execution. No compiler, solver, GUI automation
or documentation build was run during the documentation-only pass.
