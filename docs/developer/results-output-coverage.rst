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

The Boost FRD reader assembles complete ASCII logical rows before dispatch.
A ``-1`` line supplies the reported node identifier and up to six components;
``-2`` lines supply further components without repeating the identifier. A row
stays together across bounded batch boundaries. Its final component count must
match the component headers, excluding the FRD ``ALL`` display entry. This
includes ``PSTRESS``/``PCONTAC`` magnitude/phase fields and multi-line ``SDV``.
Binary FRD is explicitly rejected rather than interpreted as ASCII. The current
tokenizer supports at most 128 fields per logical record including identifiers.

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
table owns its copied buffers. ``no_partial`` omits an entire incomplete table
instead of removing individual records.

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
New datasets are optional when loading older saves in the current workspace
schema; absent analysis identity
stays unknown. New table schemas are validated before replacing loaded data.

Limits and regression checks
----------------------------

Expanded shell/beam/layered integration-point geometry remains unsupported.
Describable unexpanded geometry does not establish its DAT point mapping. The
existing solid quadrature rules, weights, interpolation and Exodus/CalculiX/FRD/
VTK node permutations are unchanged. Surface tables retain reported coordinates
without reusing volume quadrature rules.

Checks under ``tests/results`` exercise headers, continuations, variable fields,
contact identifiers and surface schemas. ``tests/vtk`` adds table ownership,
parallel equality, cancellation, invalid widths and VTT round trips. Optional
``CUBITSIM_RESULTS_PERSISTENCE_TESTS`` checks real HDF5 save/reload, invalid-schema
rejection and older-save compatibility.

``python tests/results/check_coverage.py`` performs read-only inventory and
fixture-structure checks. It does not compile or execute the C++ parsers. Native
tests and interactive job/model save/reload remain required after the user builds.
