Result readers, conversion and projection
==========================================

Registry and ownership
-----------------------

``CoreResults`` owns the per-job result registry and its FRD/DAT readers. Job ID,
result ID and reader-table index are distinct. The main result row maps these
identities. Use the query methods rather than retaining a pointer/index across
reader deletion, reload, job deletion or model reset.

Current converted results are held separately as
``shared_ptr<const SimResults::VtkResultsData>``. A published result can outlive its
registry entry only because it owns all required data; it must not borrow a
reader's vector buffer. The current build is host-thread and synchronous, with
explicit active/cancelled state and guarded reader mutation. Do not describe
that API as an asynchronous worker merely because internal conversion uses
parallel numeric work.

Source: :download:`CoreResults.hpp <../../src/CalculiX/Core/results/CoreResults.hpp>`,
:download:`VtkResultsData.hpp <../../src/Framework/Results/VtkResultsData.hpp>`
and :doc:`vtk-conversion`.

FRD and DAT have different contracts
-------------------------------------

``CoreResultsFrd`` handles FRD geometry/field blocks and their increments,
components and values. ``CoreResultsDat`` handles requested text histories,
including nodal, element and integration-point data according to the emitted
output. Presence in one reader does not imply the other format supplies the
same quantity, association or precision.

Result queries must specify the intended block/component/set and time or total
increment. The last data encountered in a file is not automatically the last
physical loading step for every quantity. Topology has explicit final-result
selection rules. Unit dimensions, tensor shear convention and node ordering
must be established before contracting or interpolating result components.

Legacy reader paths use a project ``ThreadPool``; current conversion has its own
plan/task/publication contracts. Reader thread count is separate from solver
thread count. Do not diagnose a solver issue solely from a reader timing line.
No multithreading or shutdown behavior was exercised during this documentation
pass. The utility's API hazards are recorded in :doc:`review-findings`.

Native VTK conversion
----------------------

Conversion prepares a validated plan before building datasets. Mesh provenance
can be current model, saved job input or FRD-only geometry. Single-grid and
collection layouts represent different grouping needs. Fields retain their
association, component names, source, dataset identity and validity information.
Node, element, integration-point and table data must not be merged solely because
their vector lengths happen to match.

``SimResults::Increment`` holds owning dataset/table handles. Builders must use
VTK-compatible deleters that release a reference with ``Delete()``, not default
``delete`` on a borrowed object. Published buffers are treated as immutable;
deformation/filtering should produce separate outputs. All requested tasks must
join and all requested increments validate before publication.

Conversion status distinguishes not converted, planning, planned, converting,
complete, failed and cancelled. Complete in-memory conversion is separate from
successful file serialization and external viewer launch. See :doc:`vtk-conversion`
for element/integration rules, task boundaries, options and tests.

Partial data and linking
-------------------------

Linking requires compatible identity/connectivity assumptions; matching array
counts alone is not a proof that a remeshed model is the original analysis mesh.
Partial conversion can retain available fields when some model entities have no
matching results. Missing numeric values can use NaN plus validity metadata;
they are not valid zeros. Visualization behavior depends on the selected array,
lookup table and filtering. A retained cell with unavailable values must not be
reported as a solved zero-stress or zero-displacement region.

Deleting topology removal candidates can leave model nodes or entities absent
from a later solver deck. That is a linking/provenance concern, not permission
to relabel result IDs by vector position. Keep the corresponding input/FRD/DAT
files and inspect which geometry the converter used.

Projection and topology application
------------------------------------

``CoreResultsProject`` contains displacement projection and topology application.
Displacement projection uses a selected result increment and scale; it modifies
the model representation rather than creating a new verified analysis.
Topology projection classifies a chosen stored iteration by threshold, excluding
preserved elements from removal. Preview draws retained elements/removal edges;
actual application deletes the selected volume elements through commands.

There is no mesh restoration point for topology deletion. Preparation validates
the operation, but a failed later batch can leave earlier deletions applied.
Result history is retained for numerical inspection; it does not reconstruct
deleted elements. A subsequent ordinary analysis of the changed mesh is a new
verification task. See :doc:`topology-results` for exact failure and mesh-identity
limits.

Legacy paths and external tools
--------------------------------

The ``results/legacy`` directory contains the retained legacy VTK writer. It is
excluded from the current build and has no active callers. Current conversion
uses the native plan/build/publication/export path documented in
:doc:`vtk-conversion`. Catalogue inclusion records retained source, not an
available runtime alternative.

CGX/ParaView opening submits an external process/tool request. Jobs Monitor can
offer those actions independently of topology-specific controls. The project's
CGAL and GIF adapters consume current selected geometry/frames; their external
library implementations remain outside this documentation scope.
