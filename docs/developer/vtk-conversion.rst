VTK conversion foundations
==========================

Stages 1--5 provide ownership contracts, linking/planning, element mathematics
and complete in-memory datasets built with bounded Boost tasks. A separate XML
exporter and synchronous job-completion integration are available.
The old writer is retained as ``CoreResultsVtkWriterLegacy`` under
``src/CalculiX/Core/results/legacy``. It is intentionally absent from the build
and has no active callers. Use ``CalculiXResultsInterface::build_vtk_result`` for
synchronous in-memory conversion and ``export_vtk_result`` for optional saving.
No postprocessing workspace or viewport template is introduced.

Ownership and interfaces
------------------------

``Framework/Results/VtkResultsData.hpp`` defines the solver-independent result
contract. A completed result owns all increments and optional non-spatial tables.
Unchanged mesh arrays may be shared between increments. Publish an immutable
shared handle only after all conversion tasks have joined and validation passes.
The builder supplies VTK-aware owning deleters. A borrowed VTK pointer must never
be placed in a default-deleting shared pointer. Coordinates and connectivity are
shared across increments; field arrays are owned independently of the readers.

``CalculiXResultsInterface::prepare_vtk_plan`` exposes planning without exposing
the reader containers. Planning status is separate from link source and layout.
The model and readers must remain unchanged throughout planning. The plan owns
its mesh/groups, but its field indices refer to that exact reader generation.
The core guards reader creation/deletion/reload/reset, including persistence
replacement, throughout the synchronous build. These are host-thread interfaces;
the public legacy reader containers must not be mutated directly. Job completion
loads both readers and builds the complete result before another managed job can
start. Reentrant job starts, resets and deletion are rejected during preparation.
Conversion failure/cancellation retains solver-success status but prevents the
next managed job until conversion is retried or the affected job is deleted.
Optional file-export failure does not invalidate a successfully converted result.

Commands and export
-------------------

``ccx result convert job <id>`` builds and exports all reader sources by default.
``in_memory`` builds without writing files. ``frd`` or ``dat`` selects a source;
both flags include both. ``no_partial`` skips incomplete fields. Explicit group
IDs are checked against the linked run mesh rather than the current live model.
Empty selections retain all groups of that kind.

``Framework/Results/VtkResultsExporter`` only consumes completed datasets. It
writes one VTU or VTPC per increment, VTT tables and a ``results.pvd`` time-series
index. VTPC child files are managed by VTK. Export uses a private staging
directory and publishes by renaming it after success. Destinations must be new;
existing exports remain untouched. Cancellation is checked between writer calls.
Default job exports go to ``<job-name>.vtk/run_<token>/results.pvd``. The current
export path is exposed through the results interface and used by the existing
ParaView launcher; that transient path is not persisted in the job registry.
Files remain independently usable after closing Cubit. After reloading a project,
export again to populate the launcher's current path.

Dataset construction
--------------------

``CoreResultsVtkConverter`` consumes a fresh plan without querying Cubit. It
constructs compact unstructured grids for blocks, nodesets, surfaces, DAT node
and element sets, and DAT integration points. Surface connectivity uses explicit
CCX side tables with quadratic edge nodes and outward solid-face winding.
Integration-point datasets use the union of ``(element ID, point ID)`` across
their increments. Original IDs, side labels, group names and source metadata are
retained. Unconnected result nodes are preserved as vertices; synthetic vertex
cells without an original FE element have ``OriginalElementId=-1``.

Each increment owns a grid or a partitioned dataset collection matching the
planned layout. Collections include a data assembly with stable dataset indices.
Field metadata identifies the dataset, source, scope, component labels, unique
array name and validity-mask array. The length-prefixed array names avoid
collisions between FRD/DAT fields and user set names.

Missing/non-finite tuples are represented by NaN and a zero validity mask when
``include_partial`` is enabled. Otherwise the incomplete field is skipped and a
diagnostic is retained. ``Partial`` cell arrays summarize missing incident point
or cell values. Duplicate source identifiers are rejected until an explicit
reduction policy is defined. No field is silently overwritten.

Supported integration-point positions use cached shape coefficients. Matching
FRD nodal displacement is interpolated into a separate vector array for each
increment, including when only DAT fields were requested. Missing displacement
is reported and reference geometry is retained. No deformation scale is baked
into the coordinates. Existing DAT buckling and section records become tables.

Parallel construction and publication
-------------------------------------

``CoreResultsVtkPipeline`` keeps one Boost.Asio pool for the whole build. Coordinate,
connectivity, field, integration-point displacement and partial-cell tasks write
exclusive ranges of preallocated numeric buffers. VTK object mutation and Cubit
progress/error handling remain on the calling thread. Pending work and estimated
scratch storage are bounded; mesh indexes and the complete resident output are
not subject to that temporary-work budget. All tasks join before return/cleanup.

``CoreResultsVtkOptions`` defines chunk sizes and budgets. A zero worker count
inherits the existing converter-thread setting in the core. The current readers
retain their nested numeric vectors, so conversion copies into contiguous VTK
arrays rather than claiming zero-copy access to reader storage.

``build_vtk_result`` publishes a new immutable shared result only after all
increments pass tuple-count validation. ``get_vtk_result`` returns the published
handle. ``cancel_vtk_build`` requests cancellation; failure/cancellation leaves
the previously published result intact. Published handles also outlive later
registry deletion/reload because their arrays do not borrow reader memory.

The Windows CMake configuration prepares missing development files automatically
for Cubit's VTK **9.4.2** runtime. ``CUBIT_ROOT`` selects the host installation;
``CUBIT_VTK_SDK_DIR`` defaults to ``<build-directory>/vtk-sdk`` and can select an
external adapter. A completely new build needs no manual preparation or SDK
cache setting. ``VTK_SOURCE_DIR`` defaults
to the sibling ``../VTK-9.4.2`` directory under Git. The sources stay there; no
VTK sources or runtime DLLs are copied into this repository or deployment.

During the user's initial CMake configuration,
``tools/vtk/inspect_cubit_vtk.py`` records PE exports, dependencies, architecture,
namespace evidence and DLL hashes. An optional probe using Cubit's Python records
ID width and available runtime metadata. CMake then invokes the Python preparation
helper with the selected MSVC librarian, Visual Studio instance and toolset.
It configures public sources for generated headers (including compiler probes),
then generates import libraries from Cubit's DLL exports. It does not compile
replacement VTK libraries. The standalone PowerShell helper still defaults to
writing a plan; ``-Prepare`` explicitly runs that optional manual path.

The adapter verifies the exact version, selected installation/source and DLL
hashes. Public-source settings remain candidates, not verified Coreform build
flags; C++ ABI tests are still required before compatibility can be claimed.
Use Release. The first CMake configuration performs development-file preparation;
the plugin is compiled only by the user's normal build command.
See ``tools/vtk/README.md`` for commands, output files and unresolved settings.
Linux requires an explicit host-compatible 9.4.2 SDK through ``VTK_DIR``.

``CommonCore``/``CommonDataModel`` supply conversion. ``IOXML`` and
``IOParallelXML`` supply optional VTU/VTPC/VTT serialization; IOParallelXML does
not require MPI for these local writes. The preparation helper configures those
modules and their source dependencies. Additional binary targets satisfy
references in Cubit's existing SDK exports. No postprocessing workspace,
viewport template or Qt-VTK integration feature is added.

Standalone verification
-----------------------

``tests/vtk`` builds the production converter into a DLL and exercises it across
a real C++ module boundary using Cubit's installed VTK runtime. Configure it
separately with ``cmake -S tests/vtk -B build/vtk-tests`` and the same
``CUBIT_VTK_SDK_DIR`` cache value. Build Release, then run
``ctest --test-dir build/vtk-tests -C Release --output-on-failure``.

Checks cover runtime version/ID width, node-order permutations, physical IP
coordinates and affine displacement, static-array sharing, ownership after
reader buffers are cleared, serial/parallel equality, partial-result NaNs/masks,
cancellation while work is active, failed publication, VTU/VTPC round trips and
table export. These checks establish compatibility for the tested data paths;
they do not prove every Coreform build flag or every CalculiX output variant.

Linking
-------

``CoreResultsVtkLink`` first compares the current model with the FRD mesh using
IDs, normalized connectivity, topology and coordinate tolerances. Extra unused
candidate nodes are allowed; analysis elements must match exactly. If the current
model cannot be linked, a matching run INP is tried before FRD-only fallback.
When both current model and INP match, the INP supplies the exact element
formulation, protecting against a changed full/reduced integration setting.

``CoreResultsInpReader`` reads nodes, elements, NSET/ELSET definitions, generated
ranges, forward set references and element-based surfaces. Includes are resolved
relative to their containing file; cycles and excessive nesting fail. Coordinate
systems, generated/copied meshes, assemblies and other unsupported mesh-changing
constructs must not be interpreted as a complete mesh. Set names are
case-insensitive. INP groups do not invent Cubit numeric IDs.

All metadata required for the layout is collected before allocating datasets.
Groups and integration-point clouds imply collection layout. DAT node/element
IDs are resolved against the shared job mesh. Ambiguous time matches fail rather
than associating results with the wrong increment. Existing DAT buckling and
section records are retained as table plans; broader reader coverage is deferred.

Node ordering and integration points
-----------------------------------

``CoreResultsVtkElements`` explicitly converts local positions between Exodus,
CalculiX INP, FRD and VTK conventions while preserving global node IDs. CCX 2.23
``frd.c`` writes HEX20 and WEDGE15 in Exodus edge order. CalculiX INP and VTK use
bottom/top/vertical edges; Exodus and FRD use bottom/vertical/top edges.
Quadratic CalculiX lines use endpoint/midpoint/endpoint instead of the
endpoint/endpoint/midpoint order used by Exodus, FRD and VTK. Side labels are
converted independently and never treated as VTK face indices.

``ElementIntegration`` retains the CCX 2.23 ``gauss.f`` full-integration rules for
C3D4, C3D6, C3D8, C3D10, C3D15 and C3D20. C3D8R uses the one-point hex rule with
reference weight 8; C3D20R uses the eight-point hex rule. Shape evaluation reuses
``ElementInterpolation`` in CalculiX order. Existing physical-weight callers
retain their full-integration contract.

For each supported one-based integration-point ID, position and displacement
are interpolated using the same shape functions:

.. math::

   x_{ip}=\sum_a N_a(\xi_{ip})x_a,\qquad
   u_{ip}=\sum_a N_a(\xi_{ip})u_a,\qquad
   x_{display}=x_{ip}+s u_{ip}.

Quadrature weights do not enter position/displacement interpolation. Unknown
formulations or out-of-range point IDs fail; there is no fallback to the origin.
FRD-only topology does not establish an exact integration formulation. Expanded
shell/beam and layered integration-point mappings remain explicit unsupported
cases in this stage, even where their unexpanded geometry can be described.

Verification limits
-------------------

The standalone C++ checks above have run against Cubit's installed 9.4.2 DLLs,
including reduced C3D8R interpolation and reference-volume sums for all supported
quadrature rules. Further coverage should include every ordering direction,
curved higher-order geometry, INP includes/sets/surfaces, current-model/INP
mismatch, field-name collisions and cancellation in each phase. Native job
lifecycle and picking/hover still require an interactive Cubit check. The wider
CalculiX 2.23 FRD/DAT output-coverage revision remains deferred.
