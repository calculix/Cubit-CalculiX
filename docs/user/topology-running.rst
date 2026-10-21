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
output requests. It uses the final available analysis step/increment for its
objective evaluation. A single physical job may contain several analysis steps.

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

The panel displays the latest available convergence and displacement
measurements. Bars indicate limit usage, so lower values are better; they are
not a percentage estimate of remaining runtime. Compliance also requires its
ordinary OC proposal residual to satisfy the density tolerance, even if all
three visible criteria have passed.

See :doc:`../commands/topology` for command syntax and
:doc:`../tutorials/index` for complete journals.
