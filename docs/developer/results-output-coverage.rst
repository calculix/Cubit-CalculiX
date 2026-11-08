CalculiX output coverage
========================

Scope and verification status
-----------------------------

The inventory below covers the 101 field/history keys currently exposed by
``CoreFieldOutputs`` and ``CoreHistoryOutputs``. It describes the reader and
conversion contract for CCX 2.23 output. A requested key is not proof that the
solver emitted a block: analysis type, material state, contact formulation,
output selection and solver aliases still apply. CCX 2.23 converts ``CEEQ`` to
``PEEQ``; no separate creep/plastic value may be invented.

The format changes and native regression checks have been written but have not
been compiled or executed. The inventory is a coverage contract, not a claim
that every analysis/output combination has passed an end-to-end solver test.

Source references are CCX 2.23 ``frd.c``, ``frdselect.c``, ``frdheader.c``,
``frdfluidfem.f``, ``printout*.f``, ``nodeprints.f`` and ``elprints.f``. Fixtures
are small examples constructed from their formats with distinct values; they
are not presented as unmodified solver-generated files.

.. csv-table:: Exposed-key inventory
   :file: ../../tests/results/fixtures/coverage.csv
   :header-rows: 1
   :widths: 7, 9, 7, 20, 57

Reader contracts
----------------

The standard symmetric eigenvalue summary written by CCX 2.23 ``writeev.f`` is
retained as a ``FREQUENCY`` step-summary table with mode ID, eigenvalue, real
angular frequency, frequency in cycles per model time unit, and imaginary angular
frequency. Equal frequencies do not merge mode rows. Complex and cyclic summary
layouts produce an explicit unsupported-layout diagnostic. Participation factors
and effective modal mass tables are not added by this change. Step summaries
retain their step identity and are published as global VTK tables without physical
time metadata. Their optional VTT export uses the existing summary-table path.
The development persistence identity now stores table scope explicitly; earlier
named-table identity records are not read. The updated native tests and Cubit
journal still need execution against a rebuilt plugin.

The dynamic kinetic-energy journal case uses free translation of a unit cube
with density 2 and initial velocity ``(1,0,0)``. Its analytical kinetic energy is
1, with constant velocity and displacement ``u_x=t``. Static cases do not request
``ELKE``.

The Boost FRD reader assembles complete ASCII logical rows before dispatch.
A ``-1`` line supplies the reported node identifier and up to six components;
``-2`` lines supply further components without repeating the identifier. A row
stays together across bounded batch boundaries. Its final component count must
match the component headers, excluding the FRD ``ALL`` display entry. This
includes ``PSTRESS``/``PCONTAC`` magnitude/phase fields and multi-line ``SDV``.
Binary FRD is explicitly rejected rather than interpreted as ASCII. The current
result cursor supports variable-width logical records within the reader byte limits.

Only blocks closed by ``-3`` are retained from interrupted FRD files. A final
unclosed block is discarded after worker tasks have joined. Malformed completed
records fail the read. Missing mesh connectivity still fails; there is no new
INP-only geometry fallback.

The Boost DAT path recognizes field headers explicitly. Association is encoded
in batches (node, element or volume integration point) rather than inferred
from value count. ``SDV`` and generalized displacement widths are fixed by the
first row and checked thereafter. Internal state variables are named ``SDV1``,
``SDV2``, etc. An unrecognized header clears the previous field state and is
retained as a diagnostic. Alternate/legacy reader entry points are not upgraded.

Non-spatial DAT records use reader-owned ``DatResultTable`` values. Contact
contributions retain repeated labels in source order. Node-to-face contact uses
``ReportedNodeId``; face-to-face penalty contact uses separate ``SlaveElementId``
and ``SlaveFace`` columns. Neither is silently converted into an ordinary mesh
node/IP association. ``CF``/``CFN``/``CFS`` statistics preserve slave/master pair
names and the four named groups of summary values.

Totals, inertia and centre-of-gravity results have named tables. ``FLUX`` rows
retain element, face, face integration point, flux and reported ``X/Y/Z``.
FEM-CBS ``DRAG`` rows retain element, integration point, face, stress components
and reported coordinates: its local index order differs from ``FLUX`` in CCX
2.23. Surface force/flux totals remain separate tables. No contribution
reduction or averaging occurs during reading.

Existing surface section statistics and buckling tables retain their established
storage/query path. Interrupted trailing section/contact statistics are discarded
with a diagnostic. Unsupported headers never inherit preceding field buffers.

Conversion, identity and queries
--------------------------------

Mesh compatibility and exact formulations are checked before conversion. The
plan decides groups/layout and table destinations before VTK allocations. All
increments remain resident. Publication follows worker completion and final
validation; cancellation/failure retains the previous published result.

VTK collection assemblies keep unique internal ``Dataset_N`` identifiers and use
the original group/set names as their ``label`` attributes for display. Collection
``NAME`` metadata and dataset ``GroupName`` fields retain the original names.
Labels may contain spaces, Unicode, XML-special characters or duplicate names;
each assembly leaf still maps to its own dataset index. Truly unnamed groups use
their internal identifier as a display fallback. VTPC export/reload retains these
labels. Existing exports must be regenerated to receive the labels.

FRD analysis code, mode, nodal diameter, subcase and original parameter lines are
retained with the increment and saved reader data. Field matching and
displacement interpolation respect that identity. Real/imaginary and
magnitude/phase component names remain as written. DAT blocks lacking sufficient
mode identity cannot be assigned arbitrarily when multiple frames match.
PVD exports use distinct groups for known modes, preserving equal-frequency
results without inventing a different time value.

New DAT tables attach to matching increments. VTK columns are ``RecordIndex``,
the named source columns and ``Valid``. Field data stores job/run identity,
source path/line/index, time, step/increment and available set/pair names. Each
table owns its copied buffers. Every table record is retained, including invalid
rows; the validity column supports filtering without discarding source data.

The existing exporter writes ``increment_N_table_M.vtt`` beside ``.vtu``/``.vtpc``
files and registers them in ``.pvd``. Export remains optional; tables are also
directly available from the complete in-memory result. No workspace or viewport
is created.

Raw DAT tables are available through ``CalculiXDatInterface`` and the existing
core/query facade:

* ``dat_get_table_metadata(job_id)`` returns descriptions without rows; its
  vector index is the table ID for that reader generation.
* ``dat_get_table_row_count(job_id, table_id)`` returns the row count.
* ``dat_get_table_rows(job_id, table_id, offset, count)`` returns a bounded page
  in source order. Callers must stop using table IDs after reader reload.

Save/reload includes named DAT tables, diagnostics and FRD analysis metadata.
Named-table metadata now includes the coordinate system. This is a development
format change: earlier six-string named-table metadata is not read. New table
schemas are validated before replacing loaded data.

Wide fields, local vectors and invalid values
-------------------------------------------

Logical FRD result rows and DAT field rows use a non-owning token cursor instead
of a 128-token buffer. SDV width is determined by the declared FRD components or
the first DAT row. Physical-line, decoded-row, batch and pending-memory bounds
still apply; accepting more components does not permit unbounded records.

CCX 2.23 nodal displacement, force and velocity rows ending in ``L`` retain
``NodeId`` and their three reported components in a named table, with
``CoordinateSystem=Local`` and source/set/step/increment/time provenance. No
basis transformation is inferred. In mixed blocks only global rows are mapped
to spatial vectors; all-local blocks produce no spatial vector field. Other
local/generalized layouts remain unsupported rather than being guessed.

Result payloads retain ``NaN`` and signed infinity, including finite components
next to invalid components. VTK validity is zero for a tuple containing a
nonfinite component. Missing tuples still use NaN. IDs, times and coordinates
remain finite. Standard frequency-summary metadata remains finite as well.
Stress/strain tensors containing nonfinite components produce NaN for all six
derived components and never enter the principal-value solver. Interpolated
quantities remain unavailable where their inputs are invalid.

Limits and regression checks
----------------------------

Successful VTK builds print total elapsed conversion milliseconds and phases for
planning/linking, geometry and dataset setup, field mapping (including lookup
construction), integration-point displacement interpolation, tables and final
assembly/validation/worker joining. These are sequential caller-side wall-clock
durations including worker waits, not aggregate worker or CPU times. Small setup,
publication and teardown costs can make the total exceed the sum of the phases.
Optional phase measurements are returned by the converter; Cubit logging remains
in ``CoreResults``. Failure/cancellation does not print successful-build timings.

VTK file export prints its own elapsed milliseconds, separately from conversion.
``ccx result convert job <id>`` therefore prints conversion timings
without an export duration. Explicit conversion and automatic job conversion use
the same Core timing path. Reader timings remain separate.

Expanded shell/beam/layered integration-point geometry remains unsupported.
Describable unexpanded geometry does not establish its DAT point mapping. The
existing solid quadrature rules, weights, interpolation and Exodus/CalculiX/FRD/
VTK node permutations are unchanged. Surface tables retain reported coordinates
without reusing volume quadrature rules.

Checks under ``tests/results`` exercise headers, continuations, variable fields,
contact identifiers and surface schemas. ``tests/vtk`` adds table ownership,
parallel equality, cancellation, invalid widths and VTT round trips. Optional
``CUBITSIM_RESULTS_PERSISTENCE_TESTS`` checks real HDF5 save/reload, invalid-schema
rejection and development-format round trips.

``python tests/results/check_coverage.py`` performs read-only inventory and
fixture-structure checks. It does not compile or execute the C++ parsers. Native
tests and interactive job/model save/reload remain required after the user builds.
Regressions include 126/127/512-component SDV rows, local/global mixtures,
nonfinite payload validity and local-table save/reopen.
