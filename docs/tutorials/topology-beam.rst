Beam topology: from model to retained mesh
==========================================

This tutorial uses the existing displacement beam example to prepare a model,
run density optimization, compare iterations, export views and a wrapped
surface, and optionally delete low-density elements. It explains what to look
for rather than prescribing a final iteration or promising convergence.

The preparation journal is
:download:`simple_beam_displacement.jou <../../examples/topology/simple_beam_displacement.jou>`.
It creates a topology case with a maximum of 50 iterations. Its run command
is commented out: running the journal prepares the model but does not start
optimization. The steps below start the run explicitly.
See :doc:`../user/topology-solver` for the supported analysis and required
outputs, and :doc:`../developer/topology-export` for annotated input excerpts.

.. warning::

   The preparation journal resets the current model. Save work you need before
   playing it. The optional deletion stage changes the mesh and has no automatic
   restoration point or rollback.

1. Prepare the beam
-------------------

Open Cubit with Cubit-CalculiX loaded, choose a working directory for analysis
files, and play the downloaded journal using Cubit's journal playback. A
configured CalculiX solver is required for the subsequent run. The journal uses
the Spooles solver setting; use a compatible solver configuration.

The model contains:

.. list-table:: Prepared model
   :header-rows: 1
   :widths: 30 70

   * - Item
     - Definition
   * - Geometry
     - A brick with dimensions 100 by 30 by 1, with an end region split for the rigid-body selection.
   * - Mesh
     - HEX8 volume mesh in block 1, exported as C3D8.
   * - Material and section
     - Steel with elastic modulus 210000 and Poisson ratio 0.3, assigned through a solid section on block 1.
   * - Support
     - Nodeset 1, named ``x0``, with all three displacement components fixed to zero.
   * - Loaded region
     - Nodeset 3, named ``rb``, connected to reference and rotation nodes by a rigid-body constraint.
   * - Load
     - Force 10000 in the negative Y direction on the rigid-body reference vertex.
   * - Step
     - A linear static step with the force, displacement BC, and output requests assigned.

Use a consistent unit system for dimensions, material properties, and force.
Cubit-CalculiX does not infer units from these numbers. The journal also creates
nodeset 2, named ``xl``; it is not the topology displacement selection.

Display the mesh and inspect the support and rigid-body nodesets before running.
Do not substitute the separate ``simple_beam.jou`` file: this walkthrough uses
the displacement journal linked above.

2. Inspect topology settings
----------------------------

Expand **Topology** in the model tree and open case 1's settings. The journal
creates the following configuration:

.. literalinclude:: ../../examples/topology/simple_beam_displacement.jou
   :language: text
   :start-at: ccx create topology 1
   :end-at: ccx create topology 1

Check **Method: Density**, **Objective: Displacement**, **Displacement nodeset:
3**, **Displacement component: Y**, and **Volume fraction: 0.7**. Other omitted
settings use the defaults from the core; see :doc:`../user/topology-settings`.

This objective minimizes the maximum absolute global Y displacement among
nodes in nodeset 3. It is not compliance optimization with a displacement
monitor. It can require an additional adjoint job per evaluated design.
Compliance adaptive move settings do not adapt displacement updates.

3. Inspect automatic preservation
---------------------------------

Open the case's **Manage preserved sets** panel. Inspect the automatic rules
and leave them unchanged for this example. Run preparation resolves those
rules to participating elements.

The fixed displacement BC contributes its support nodeset. The rigid-body
constraint contributes the connected region, preserving elements touching
that region. The force on a free reference node need not directly select any
volume elements; the rigid-body selection protects the attached region.
Nodeset 3 is not preserved merely because the objective monitors it.

When initialization reports the resolved selections, check the support and
rigid-body region are represented. Preserved elements remain at density one
and count toward the volume target. See :doc:`../user/topology-preservation`
for overlapping sets and explicit overrides.

4. Start and monitor the run
----------------------------

Start asynchronously from the command line:

.. code-block:: text

   ccx run topology 1

Alternatively, open **Actions** for case 1, leave **Wait until finished**
unchecked, and click **Run**. The run checks the analysis and configures required
outputs before evaluating designs. An information message about automatically
configured outputs is expected when required requests or keys are missing.

The command returns control while jobs progress. Results loading can still
pause the UI; asynchronous solver execution does not make reading asynchronous.
The Actions panel reports the current state, iteration, job, and completed
iteration diagnostics. Use **Job monitor** to inspect the active physical job.

If you want to wait after starting, use:

.. code-block:: text

   ccx wait topology 1

**Kill** requests termination and retains completed iterations. It is not
pause/resume. Starting the same topology case again resets its earlier run
data, so do not press Run again simply to refresh the display.

5. Interpret the progress
-------------------------

Iteration 1 is the initial fully solid design. Its volume fraction should be
one. Subsequent move-limited updates work toward the 0.7 upper bound while
preserved regions remain solid. The reported objective can increase while
volume is removed, and displacement updates do not guarantee a decrease at
every iteration.

Use the Actions panel's convergence indicators to compare density change,
relative objective change, and volume against their limits. ``WAIT`` means a
criterion has not passed. The initial iteration has no previous design for a
convergence comparison. See :doc:`../user/topology-objectives` for the equations.

Both ``converged`` and ``iteration_limit`` end a run normally, but reaching the
iteration limit does not establish convergence. There is no expected fixed
iteration count or numerical objective specified for this walkthrough.
Inspect the actual run messages if it ends as ``failed`` or ``killed``.

6. Compare completed iterations
-------------------------------

After the run is idle, right-click a completed iteration and choose
**Browse iteration**. Use **Reload** if new results are not listed. The Results
panel initially opens a case at its last completed iteration unless another
selection was requested or retained.

Use the dropdown, navigation buttons, or slider to compare iterations. Enable
**Automatically preview iteration changes** and set the threshold to 0.5.
Retained elements are green; removal candidates appear as red edges. Preview
commands run silently without repeatedly recording them in command history.

For an explicit first-design check, run:

.. code-block:: text

   ccx result project topology 1 iteration 1 threshold 0.5 preview

Because the first design is fully solid, this preview should have no removal
candidates. Then choose a later completed iteration in the panel. Adjust the
threshold to compare its retained regions. Elements exactly at the threshold
are retained, and preserved elements are retained at every threshold.

With at least two completed iterations, use **Play/Pause** to browse the sequence
at the selected threshold. Interactive playback stops at the last iteration;
it does not run additional optimization steps.

Choose an iteration based on its objective, volume, convergence information,
and retained shape. A useful design can be an intermediate result, and a
converged result still requires engineering review. Keep the source mesh
unchanged while comparing designs.

7. Export before changing the mesh
----------------------------------

Set the desired camera and threshold before exporting. To create a looping
GIF of all completed iterations:

.. code-block:: text

   ccx animate topology 1 threshold 0.5 fps 5 save "beam.gif"

Use a different filename or add ``overwrite`` for an existing file. The Results
panel provides the same filename, frame-rate, and overwrite controls. A GIF
captures all eligible iterations, not just the displayed one.

For a wrapped surface of your chosen iteration, use the Results panel's
**Alpha wrapping** group. Leave alpha and offset empty for automatic values,
choose ``beam.stl``, clear **Import STL into Cubit**, and click
**Wrap displayed iteration**. This exports the retained region without deleting
the source volume mesh or importing the new STL.

The equivalent command template is below. Replace ``<chosen_iteration>`` with
an actual completed iteration ID before running it:

.. code-block:: text

   ccx wrap topology 1 iteration <chosen_iteration> threshold 0.5 file "beam.stl" no_import

Wrapping uses a surface approximation of the selected mesh. It does not create
a volume mesh ready for analysis or transfer the stored density stiffnesses.
See :doc:`../user/topology-results` for alpha, offset, and filename defaults.

8. Optionally apply the threshold
---------------------------------

This stage changes the mesh. Complete the comparisons and exports you need
first. Choose the desired iteration and threshold in the Results panel, click
**Preview**, and review the retained support, loaded region, and connecting mesh.

If you decide to apply that result, use **Delete below threshold**. It deletes
unpreserved elements strictly below the displayed threshold. There is no
restoration point or rollback, and deletion can leave a partly modified mesh
if a later batch fails. Stored numerical iteration records remain, but mesh
actions can become unavailable because elements no longer exist.

The command equivalent is intentionally separate from the preview/export
sequence. Replace the iteration placeholder before executing it:

.. code-block:: text

   ccx result project topology 1 iteration <chosen_iteration> threshold 0.5

9. Analyse the remaining mesh
-----------------------------

Inspect the remaining connectivity and section, material, constraint, load,
and boundary-condition assignments. A thresholded mesh can contain disconnected
or unsuitable regions; the earlier density solve does not verify this new mesh.

Create a normal job:

.. code-block:: text

   ccx create job name "beam_after_topology"

Find its assigned ID in the Jobs tree or job monitor, then run that actual ID:

.. code-block:: text

   ccx run job <new_job_id>

Job IDs are not iteration IDs. Physical and adjoint jobs created during
optimization also consume IDs, so do not assume a fixed ID for this job.
Running it exports the current analysis model. Its material assignments govern
the retained elements; browsing or deleting from an iteration does not apply
the iteration's computed stiffness factors to this new job.

Inspect the new solve and its physical displacements and stresses through the
normal results workflow. If the remaining model is unsuitable, reconsider the
chosen design or threshold using the corresponding original mesh.
