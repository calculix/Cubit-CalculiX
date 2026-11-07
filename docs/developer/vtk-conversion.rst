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
both flags include both. Incomplete fields and tables are always retained. Explicit group
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

Missing tuples are represented by NaN and a zero validity mask. Reported
non-finite values retain their components with zero tuple validity. Fields and
table rows are always retained. ``Partial`` cell arrays summarize invalid
incident point or cell values. Duplicate source identifiers are rejected until an explicit
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

Topology integration and reported formulation failure
-----------------------------------------------------

The user-supplied log reviewed on 6 November 2026 records an ordinary HEX20 job
exporting VTK successfully, followed by a HEX8 topology run whose first physical
solve finished. Its FRD reader reported no result fields; DAT reading supplied
internal energy 1237.94944 and compliance 2475.89888. Planning then reported:

.. code-block:: text

   Exact supported element formulation is required to locate DAT integration points for element 1.

The topology evaluator nevertheless completed iteration 1 and prepared the next
density design. Starting iteration 2 was rejected because job 1 still had pending
VTK conversion. These are separate reader, evaluator and publication outcomes;
the compliance value is not evidence of a completed VTK result. This is reported
runtime evidence, not a reproduction performed during documentation work.

``CoreResultsVtkLink::fields()`` emits that message when a DAT integration-point
field references an element with an empty, unsupported or non-integrable
``calculix_type``. ``prepare()`` first requires a valid FRD mesh. It attempts a
current-model match, augments a match with exact formulations from a compatible
INP, then tries INP linking before retaining FRD-only geometry. FRD-only element
topology cannot distinguish full from reduced integration. Even a DAT-only
conversion request currently passes through this FRD mesh requirement.

The log does not identify which match failed or prove that C3D8 itself is
unsupported. It does not contain the actual INP element declarations, link
diagnostics or FRD connectivity needed to distinguish those causes. Preserve the
run INP and included files, FRD/DAT, working directory, link diagnostics and exact
formulations before changing output requests or model geometry. A no-fields FRD
message alone does not establish an empty mesh. Partial-field options do not
relax the requirement for valid integration-point geometry.

After the cause is resolved, ``ccx result convert job <id> in_memory`` retries
publication without file export. It can clear the next-job gate, but does not
restart a failed topology controller. Review items and regression expectations
are recorded in :doc:`review-findings` and :doc:`topology-verification`.

Node ordering and integration points
------------------------------------

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

FRD contact-contribution tables
--------------------------------

FRD ``CELS`` blocks are retained as one ``vtkTable`` per source block in the
corresponding increment's ``tables`` collection. Columns are ``RecordIndex``
(zero-based source row), ``ReportedNodeId`` (the FRD label), ``CELS`` (the
unreduced energy) and ``Valid`` (one for finite energy, zero otherwise). Repeated
labels remain separate rows in source order. They do not pass through the
spatial-field duplicate-ID lookup.

``RecordIndex`` identifies a row within one source block, not a contact point
across increments. A reported node label alone does not provide the actual
contact integration-point coordinates or a unique contact-element identity.
No averaging or summation is applied. Non-finite values and all source records
remain in the table, with ``Valid=0`` on invalid rows.

Table field data records the job/run identity, source file/block/data indices,
step/increment, time, and the identifier and reduction conventions. The result
owns its columns independently of the readers. The existing exporter writes
``increment_N_table_M.vtt`` and registers it in the PVD index under
``IncrementTables``. Global DAT buckling and section tables retain their existing
export path. No viewport or postprocessing workspace is introduced.

The CELS checks cover repeated labels, changing row counts, non-finite energies,
whole-table omission, serial/parallel values, reader-buffer lifetime and VTT
export. These additions have not been compiled or run; building remains a user
action.

Verification limits
-------------------

The standalone C++ checks above have run against Cubit's installed 9.4.2 DLLs,
including reduced C3D8R interpolation and reference-volume sums for all supported
quadrature rules. Further coverage should include every ordering direction,
curved higher-order geometry, INP includes/sets/surfaces, current-model/INP
mismatch, field-name collisions and cancellation in each phase. Native job
lifecycle and picking/hover still require an interactive Cubit check. The
exposed-key coverage work and its remaining validation limits are recorded in
:doc:`results-output-coverage`.

Conversion performance and work budgets
--------------------------------------

``CoreResultsVtkRowMaps`` is a caller-owned cache for one build. It interns
validated sorted source layouts using association, entity/IP keys and original
source-row positions. Hashes select candidates; exact comparisons establish a
hit. Each layout caches destination-to-source row indices by immutable domain ID.
Ordered dense destinations use a merge; sparse or unsorted destinations use
binary search while constructing the map. Cache hits allow field workers to copy
through numeric indices. Source values, widths and offsets are validated for
every field and are never cached as pointers. Already sorted source layouts avoid
another sort. Missing tuples retain NaN; nonfinite payloads remain unchanged.

Before creating the pipeline, the converter reserves up to one quarter of
``pending_bytes`` (capped at 16 MiB each) for maps and reduction scratch when the
budget is at least four chunks. The scheduler receives the remainder. Smaller
budgets disable both auxiliary reservations. Cache vector capacities and estimated
list-node overhead are charged to the map budget. Existing source/mesh indexes,
allocator bookkeeping and resident result arrays remain outside this work budget.
Cache eviction happens only on the caller between completed operations. Oversized
maps use the uncached lookup path rather than exceeding the cache reservation.

Field and IP-displacement workers coalesce invalidity once per chunk. After work
completes, the caller records pointers only to incomplete masks, whose VTK arrays
are retained by the corresponding grid. Finalization reduces point masks once,
reduces cell invalidity directly into the Partial array and traverses connectivity
once. Fully valid grids fill Partial with zero without connectivity inspection.
If a point-mask union cannot fit the scratch reservation, the previous per-cell
scan of incomplete point masks remains available as a bounded-memory fallback.

The Boost pool remains reusable across all arrays. One-worker operations execute
directly in bounded chunks; multiworker operations also use a direct path for a
single chunk estimated at no more than 64 KiB. Parallel tasks notify a condition
variable on completion. A 20 ms timeout preserves owner-thread progress and
cancellation polling without mandatory 1 ms sleeps after short tasks. Workers
write exclusive numeric output ranges, never VTK/Cubit APIs. Failure/cancellation
stops and joins the pool before returning; publication remains all-or-nothing.

Existing wall-clock conversion phase timings remain the benchmark. Regression
coverage in ``tests/vtk/VtkDatasetTests.cpp`` includes source-row reorderings,
changed subsets/IP keys, cache eviction, cached versus low-budget fallbacks,
point/cell invalidity, direct/parallel execution, worker failures and cancellation.
These native tests require the user's build and were not compiled or run as part
of this change.
