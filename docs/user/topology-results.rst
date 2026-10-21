Reviewing and applying topology results
=======================================

A density iteration stores a distribution of element densities and stiffness
factors. Reviewing it does not delete elements or assign its computed
stiffnesses to a new analysis. The threshold controls a later decision about
which elements to retain. See :doc:`topology-objectives` for interpreting
optimization quality and :doc:`../commands/topology-results` for command syntax.

Finding an iteration
--------------------

The model tree organizes results as follows:

.. code-block:: text

   Topology
     Case name                         Case ID
       Iterations
         Iteration 1                   Iteration ID
           Job 1                       Physical job ID
         Iteration 2
           Job 3

Iteration IDs start at one. Job IDs need not follow the same sequence because
other jobs, including displacement adjoint jobs, also receive IDs. The job
child represents the physical solve.

Case icons distinguish not run, running, finished, killed, and failed states.
Both convergence and the iteration limit use the finished icon; inspect the
case state or Actions panel to distinguish them. Iteration icons reflect their
physical job status. A completed physical job alone does not establish that
objective evaluation has finished.

Right-click a completed iteration and choose **Browse iteration** to open the
Results panel at that iteration. **Preview (threshold 0.5)** draws a preview
directly. Activating an iteration also requests that preview. Activating its
job opens the job monitor. Case context-menu actions provide settings,
preservation management, run controls, and the Actions panel.

Navigating the Results panel
----------------------------

Choose a topology case and completed iteration. When first opening a case,
the panel initially displays its last completed iteration. **Reload** refreshes
the available cases and iterations; a reload is useful after new results arrive.
The iteration list includes evaluated iterations with a finished physical job.

Use the first, previous, next, and last buttons, the dropdown, or the horizontal
slider to move through completed iterations. The slider follows the available
entries rather than assuming that every possible iteration ID exists.

The panel reports the physical job, objective, volume fraction, density change,
relative objective change, convergence, and available displacement information.
**Job monitor** opens the displayed iteration's physical job. **ParaView** opens
that job's results through the existing result workflow.

The displayed iteration is a panel selection. It does not change the core's
stored selected iteration. The panel passes an explicit iteration ID to its
preview, deletion, and wrap commands. Commands with an omitted iteration use
the core selection instead; specify an iteration explicitly when reproducing
a panel operation in a journal.

Threshold and preview
---------------------

For threshold :math:`t` in [0, 1], an element is retained if it is preserved or
its density is at least :math:`t`. An unpreserved element with density strictly
below :math:`t` is a removal candidate. A density exactly equal to the threshold
is retained. The default threshold is 0.5.

**Preview** draws retained elements in green and removal candidates as red
edges. It changes the display, without changing the mesh. Enable
**Automatically preview iteration changes** to refresh after changing the
iteration or threshold. Preview commands run silently, avoiding repeated
entries in command history. A preview failure is still reported.

Preview and other mesh operations need the original stored elements to exist
in the current Cubit mesh. They are unavailable in the Results panel while
the topology case is running. Editing or deleting the mesh can make them
unavailable even though numerical iteration records remain visible.

Playing through iterations
--------------------------

With at least two completed iterations and usable mesh data, **Play** previews
successive iterations at the displayed threshold. **Pause** stops playback.
Starting at the last iteration restarts from the first; playback then stops at
the last iteration. Changing the iteration manually or hiding the panel also
stops playback.

**Playback / GIF FPS** controls both the panel playback interval and GIF export.
Drawing takes time, so interactive playback can be slower than the requested
rate. Playback does not run the optimizer or interpolate new density designs.

Exporting a GIF
---------------

**Export GIF** captures all completed, evaluated iterations of the case in
numerical order, using the displayed threshold. It does not export only the
displayed iteration. Set the camera before exporting: frames are rendered
through Cubit's current graphics view.

The default rate is 5 FPS, with an allowed range of 1--100. An empty filename
uses ``topo_<id>_animation.gif`` in the working directory. Choose a filename
ending in ``.gif``; its parent directory must already exist. Existing files
require **Overwrite existing GIF**. The exported GIF loops, whereas panel
playback stops at the final iteration. Export previews each iteration and
can leave the graphics view showing the final captured design.

Wrapping an iteration
---------------------

The **Alpha wrapping** group exports an STL surface around the retained mesh
of the displayed iteration. Preserved elements are always included. Wrapping
uses current Cubit coordinates and does not delete the source volume elements.

Leave **Alpha** empty to use the input bounding-box diagonal divided by 40.
Leave **Offset** empty to use the resolved alpha divided by 30. Explicit values
must be finite and positive, in model length units. These parameters control
the wrapping operation; the density threshold selects its input elements.

An empty panel filename uses ``topo_<id>_iteration_<iteration>_wrap.stl``.
The command form always requires an explicit filename. **Import STL into
Cubit** is enabled by default; clear it to export only. Existing files require
**Overwrite existing STL**. Use **Wrap displayed iteration** to run the command.

Higher-order elements use a corner-node surface approximation for wrapping.
The STL is a wrapped surface, not the original volume mesh or a new analysis
with the stored density-dependent stiffnesses.

Deleting elements below the threshold
-------------------------------------

Review the desired iteration with the desired threshold, then use
**Delete below threshold** to delete eligible volume elements from the current
mesh. Preserved elements are excluded. This does not create a restoration
point or roll back deletion on failure. A failure after deletion has started
can leave part of the mesh deleted.

The panel refreshes availability after the command, including after failure.
Historical records remain stored, but further preview, playback, GIF, wrapping,
or deletion can fail or become disabled because stored elements are missing.
Export the views or wrapped surfaces you want before applying deletion.

To analyze the remaining mesh, check its mesh, sections, loads, boundary
conditions, and constraints, then create and run a normal job. That analysis
uses its current assignments; the preview threshold does not transfer the
iteration's computed element stiffness factors to it.

For a command sequence with explicit iteration IDs, see
:doc:`../commands/topology-results`. For persistence and validation details,
see :doc:`../developer/topology-results`.
