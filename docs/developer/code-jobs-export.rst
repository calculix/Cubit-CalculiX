Job processes and input-deck export
===================================

Separate lifecycle stages
--------------------------

Job creation records settings. Running a job prepares/writes its input and starts
the solver process. Process completion, reader completion, converted-result
publication and external viewer opening are later events. Do not interpret one
status as proof that all subsequent stages succeeded.

``CoreJobs`` owns job records and platform process/pipe state. Commands and GUI
use ``CalculiXJobsInterface`` and existing coordination methods. The typed
``JobsSettings`` read view is preferable to interpreting table offsets in a new
consumer. Export can be invoked explicitly, but normal job execution already
uses the established input-preparation path.

Source: :download:`CoreJobs.hpp <../../src/CalculiX/Core/jobs/CoreJobs.hpp>`
and :download:`CoreJobs.cpp <../../src/CalculiX/Core/jobs/CoreJobs.cpp>`.

Job records and states
-----------------------

The main legacy job row contains job ID, name, path, status, process ID, console
record ID, conversion state, convergence/status-file references and automatic
conversion choice. These values are not interchangeable with OS handles or
indices in the vector. Use ID lookup/query methods before accessing a record.

.. list-table:: Status query values in CoreJobs
   :header-rows: 1
   :widths: 20 80

   * - Value
     - Meaning
   * - -2
     - No matching job, as returned by the status query.
   * - -1
     - No process associated with the job.
   * - 1
     - Process running.
   * - 2
     - Process finished.
   * - 3
     - Process killed.
   * - 4
     - Process finished with errors.
   * - 5
     - Process running while being waited on.

Topology has a separate run-state enum: not run, running, converged, iteration
limit, failed and killed. An optimization run can have several job phases.
Never reuse the job status integer as a topology run-state value.

Run, wait, kill and completion
------------------------------

``run_job()``, ``wait_job()``, ``kill_job()`` and ``check_jobs()`` coordinate
processes and subsequent result work. Windows maintains process handles and
pipe-reader threads; Linux uses the Cubit process adapter. A saved process ID is
not sufficient to reconstruct those resources after load.

The completion path guards against recursive result completion and can keep
pending result jobs. A nonblocking solver start does not make result reading
asynchronous. Wait can process host events; reentrant reset/deletion while a
completion operation owns reader/model data must respect the existing guards.
Kill request acceptance and confirmed terminal state are separate observations.

``.sta`` and ``.cvg`` data are progress/status evidence; ``.frd`` and ``.dat``
contain numerical results used by readers. External files must remain available
for workflows that re-read them. A model save does not automatically package
the whole job directory or make absolute external paths portable.

Structured export
-------------------

``ccxExportCommand::execute()`` establishes the export context and resources;
``write_file()`` orchestrates the deck. Dedicated writers handle nodes,
connectivity, sets, materials, sections, constraints, surface interactions,
contact pairs, amplitudes, orientations, damping, physical constants, initial
conditions, homogeneous conditions and steps. Follow the actual call order in
``write_file()`` when adding a dependency between emitted definitions.

The material/section writers receive explicit topology/iteration context for
iteration jobs. Normal exports use their normal context. Do not rebuild the
entire export pipeline in an optimization controller or inject a special partial
step writer without checking the established validation/preparation path.

Source: :download:`ccxExportCommand.hpp <../../src/CalculiX/Commands/export/ccxExportCommand.hpp>`
and :download:`ccxExportCommand.cpp <../../src/CalculiX/Commands/export/ccxExportCommand.cpp>`.

Resource and failure contracts
-------------------------------

Mesh/material interfaces are acquired from the host. Progress tools can be
borrowed. Scope guards release/clean export preparation and restore temporary
host options; they are not permission to destroy borrowed objects. An exception
while querying an element should be associated with its pipeline stage and
identity to make a failing deck diagnosable.

An empty export string may mean no optional data or a validation error, depending
on the writer. Preserve its existing return/error checks. A false return does
not prove that no bytes were written; partially prepared/written files must not
be mistaken for completed solver inputs. Existing overwrite behavior belongs
to the particular command and destination handling.

Custom lines
--------------

``CoreCustomLines`` stores user-specified text at its supported export insertion
locations. These lines are solver input, not validated model features with all
the same typed settings and relationship checks. Document the insertion point
and the user's responsibility for keyword compatibility. Custom text does not
automatically become a visible load/BC/constraint for topology preservation.

Topology jobs
--------------

Each evaluated design owns an iteration job; displacement objectives can also
own an adjoint job. One-based iteration IDs and independent job IDs are retained
for browsing. Case naming and topology IDs contribute to generated job names.
Rerun/reset removes the case's run records and associated jobs through the
existing lifecycle, rather than resuming a selected result. Preview and deletion
remain explicit user actions. See :doc:`topology`, :doc:`topology-export` and
:doc:`topology-results` for the specialized contracts and mathematical limits.
