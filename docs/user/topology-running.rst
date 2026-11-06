Running a topology case
=======================

Preparing the analysis
----------------------

Prepare the mesh, CalculiX element types, materials, solid sections, loads,
boundary conditions, and analysis steps before starting optimization.
Loads and boundary conditions must be assigned to the intended analysis steps.
For displacement optimization, select an existing, nonempty displacement
nodeset and a global component or magnitude; see :doc:`topology-settings`.

The run validates the supported analysis and objective, resolves preserved
elements, initializes the density design, and checks/configures the required
output requests. The evaluator requires its quantities at the final DAT
step/increment recorded by the parser. Missing data there is an error; it does
not fall back to an earlier frame. A single physical job may contain several
analysis steps. Physical and adjoint files select their final frames independently.

Starting asynchronously
-----------------------

.. code-block:: text

   ccx run topology 1

The command starts case 1 and returns control while solver jobs progress.
The topology controller advances the run as jobs are polled and their results
become available. A successful asynchronous start does not mean optimization
has converged or even completed its first iteration.

Asynchronous solver execution does not make result reading asynchronous.
Cubit may still pause while loading and evaluating results.

Result conversion between solves
--------------------------------

The current job-completion routine reads FRD/DAT and builds an in-memory VTK
result before another managed job may start. The automatic file-export setting
controls writing the converted result; disabling it does not skip the in-memory
build. This applies to physical and adjoint topology jobs as well as ordinary
jobs.

A finished solver process and a printed compliance value do not establish that
conversion succeeded. If conversion fails, the job can remain solver-finished
while the next iteration is blocked by pending VTK conversion. The message
``results are loaded`` currently describes topology evaluation, not successful
VTK publication.

Keep the matching INP, FRD and DAT files. DAT integration-point visualization
requires an exact supported CalculiX formulation, including full/reduced
integration; a generic hex, tet or wedge shape does not establish it. Missing
FRD field output is distinct from missing FRD mesh geometry. The converter
currently requires FRD geometry even when converting DAT fields.

After resolving the reported conversion problem, retry the affected job, for
example:

.. code-block:: text

   ccx result convert job 1 in_memory

Use the actual failed job ID. Retrying with the same unresolved inputs can fail
again. A successful retry clears the conversion prerequisite on the next job
readiness check; it does not resume a topology run already marked failed.
Starting that topology case again resets its previous run data as described
below. Deleting an affected job also removes its pending conversion entry, but
deletion is not a conversion repair and can affect retained topology history.

Waiting for completion
----------------------

To start and wait in one command:

.. code-block:: text

   ccx run topology 1 wait

To wait after an asynchronous start:

.. code-block:: text

   ccx run topology 1
   ccx wait topology 1

Waiting processes job updates until the current run finishes. Convergence or
the iteration limit is a successful completion. A failed or killed run is not
a successful wait result. Waiting requires a current run context; loading a
saved case does not recreate an active controller context. Nested topology
waits are rejected.

Stopping a run
--------------

.. code-block:: text

   ccx kill topology 1

Kill requests termination. The controller stops the active physical or adjoint
job and ends the run once termination has been confirmed. Request acceptance
does not guarantee immediate process termination. If termination is pending,
the controller reports that condition and checks again.

Completed iteration results are retained. An interrupted physical or adjoint
solve is not a completed, evaluated topology iteration. Kill is not pause/resume;
starting the case again starts a fresh run.

Understanding run states
------------------------

.. list-table:: Case run states
   :header-rows: 1
   :widths: 25 75

   * - State
     - Meaning
   * - ``not_run``
     - No run has started, or the case's run data has been reset.
   * - ``running``
     - Preparation, solving, result processing, or updating is active. A requested stop can still be pending.
   * - ``converged``
     - The implemented convergence checks passed.
   * - ``iteration_limit``
     - The maximum evaluated iteration count was reached before convergence.
   * - ``killed``
     - The run ended following a user stop request.
   * - ``failed``
     - Preparation, solver execution, result processing, or evaluation failed.

``iteration_limit`` is not a solver failure and does not imply convergence.
An iteration can also show ``WAIT`` for a convergence criterion while its
physical and adjoint solves have both completed successfully.

Understanding iteration jobs
----------------------------

Iteration IDs begin at 1. Each design receives its own physical job. A nonzero
displacement objective also receives an adjoint job; a zero objective uses the
implemented zero subgradient and skips that solve.

Names follow this pattern:

.. code-block:: text

   topo_<case_id>_<case_name>_iter_0001
   topo_<case_id>_<case_name>_iter_0001_adjoint

The name portion replaces characters other than ASCII letters, digits, and
underscores with underscores. The numeric iteration suffix is padded to at
least four digits. There is no template job and no separate directory per
iteration. Ordinary job working-directory and solver settings apply.

Starting the same case again
----------------------------

Starting again clears this case's previous iteration job records, evaluations,
density designs, resolved element-preservation flags, selected iteration, and
prepared topology materials/sections. Case settings and user preservation
selections remain. The next run resolves the model and selections again.

This is a new run, not a continuation from the selected iteration. The plugin
does not preserve the previous iteration tree when restarting the same case.
Save the model and copy any solver files you want to retain before restarting;
job filenames derived from the same case name and iteration may be reused.

Using the Actions panel
-----------------------

.. list-table:: Controls
   :header-rows: 1
   :widths: 30 70

   * - Control
     - Action
   * - Topology case
     - Selects the case to operate on.
   * - Reload
     - Refreshes the case list and displayed state.
   * - Run
     - Starts a fresh run; disabled while that case is active.
   * - Wait until finished
     - Adds ``wait`` when Run is pressed. Set it before starting.
   * - Wait
     - Waits for the active run; disabled during an existing wait.
   * - Kill
     - Requests termination; disabled after a stop has already been requested.
   * - Job monitor
     - Opens the monitor for the controller's current associated job, which can be physical or adjoint.

The Actions panel and Topology Management's Run tab display the latest
available convergence and displacement measurements. Their three bars show
maximum density change, relative objective change and volume fraction. Bars
indicate limit usage, so lower values are better; they are not a percentage
estimate of optimization completion or remaining runtime. Both objectives also require an
ordinary proposal residual to satisfy the density tolerance: OC for compliance,
projected-gradient for displacement. This can prevent convergence even when all
three visible criteria pass. The proposal uses the configured maximum move,
independently of adaptive restriction, and requires no additional solve. It is
a fourth numerical check, not a fourth bar. If the three bars pass but that
proposal has not settled, the summary still reports ``Converged: NO``. See
:doc:`topology-objectives` for the criteria and their interpretation.

See :doc:`../commands/topology` for command syntax and
:doc:`../tutorials/index` for complete journals.
