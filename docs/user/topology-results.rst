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

Double-click **Topology** in the model tree to open **Topology Management**.
Double-click a case to open it with that case selected. Double-click an iteration,
or choose **Browse iteration** from its context menu, to open the **Iterations**
tab at that iteration. Activating a physical or adjoint job opens Jobs Monitor.
**Preview (threshold 0.5)** remains available in the iteration context menu.
There is no management navigation node and no separate Results command panel.

Navigating Topology Management
------------------------------

Select a case in the left-hand list. The right-hand tabs contain Settings,
Preserved sets, Run, Iterations, GIF, and Alpha wrap. See
:doc:`topology-management` for case editing and execution.

The **Iterations** table shows physical-job states and stored evaluations.
The completed-iteration dropdown initially selects the last available evaluation.
Refresh retains an existing displayed iteration as new results arrive. Use the
first, previous, next, and last buttons, the dropdown, or the horizontal slider
to move through completed iterations. Slider positions follow available entries,
not an assumption that every iteration ID exists.

Result-iteration and threshold controls are inside their relevant tabs. The GIF
tab has a threshold and exports all completed iterations; it has no iteration
selector. The Iterations and Alpha wrap tabs synchronize their displayed iteration
and threshold values within the window. **Job monitor** opens the physical job; **Adjoint job
monitor** opens the associated adjoint when available. ParaView remains available from Jobs Monitor; Topology Management has no ParaView button.

The displayed iteration is a GUI selection. It does not change the core's
stored selected iteration. Preview, deletion and wrapping commands include an
explicit iteration ID. Specify that ID when reproducing an operation in a journal.

Threshold and preview
---------------------

For threshold :math:`t` in [0, 1], an element is retained if it is preserved or
its density is at least :math:`t`. An unpreserved element with density strictly
below :math:`t` is a removal candidate. A density exactly equal to the threshold
is retained. The command and management default threshold is 0.5; the toolbar
can restore its last saved GUI threshold preference. This threshold is separate
from the optimization volume target, void stiffness ratio and minimum density.
Changing it does not rerun the optimizer or modify the stored densities or
stiffness factors. See :doc:`topology-settings`.

**Preview** draws retained elements in green and removal candidates as red
edges. It changes the display, without changing the mesh. Enable
**Automatically preview iteration changes** to refresh after changing the
iteration or threshold. Preview commands run silently, avoiding repeated
entries in command history. A preview failure is still reported.

Preview and other mesh operations need the original stored elements to exist
in the current Cubit mesh. They are unavailable in the Topology Management while
the topology case is running. Editing or deleting the mesh can make them
unavailable even though numerical iteration records remain visible.

Playing through iterations
--------------------------

With at least two completed iterations and usable mesh data, **Play** previews
successive iterations at the displayed threshold. **Pause** stops playback.
Starting at the last iteration restarts from the first; playback then stops at
the last iteration. Changing the iteration manually or hiding the window or leaving the Iterations tab also
stops playback.

**Playback / GIF FPS** on the GIF tab controls both interactive playback and GIF export.
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

Export starts asynchronously. Frame capture stays on the GUI thread and yields
between frames; image encoding, publishing, and temporary-file cleanup run on a
worker. A complex frame can still briefly pause the UI. The final saved-file
message appears after cleanup, rather than before deleting temporary PNGs.

Use **Cancel GIF export** to stop between frames. Cancellation during encoding
can wait for the current frame; if publication already completed, the output
remains. While export is active, the Topology Management disables competing preview,
playback, navigation, threshold, deletion, and wrapping actions. Avoid changing
the model or graphics view through other commands during capture. Model reset
or load requests cancellation. Only one GIF export can be active, and starting
a topology solve is rejected until export ends.

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


Postprocessor iteration series
------------------------------

The Postprocessor source dropdown lists each CalculiX topology case once.
Physical iteration jobs and adjoint jobs remain available in CalculiX, and you
can still open a child explicitly with ``post open ccx job <id>``. A case
without completed physical evaluations is disabled in the source catalogue.

Open a topology case from the Postprocessor or use::

  post open ccx topology 1
  post open ccx topology 1 threshold 0.65 name "Beam at 0.65"

Use **Open Results** beside **Job monitor** on the Topology Management **Run**
tab to open the case series. **Open Results** on **Iterations** opens the same
series at the selected completed iteration. These actions work for completed
iterations of a stopped case. The incomplete iteration of a killed job is
excluded. The Iterations action submits ``post open ccx topology 1 iteration 7``
for case 1, iteration 7.

Each completed physical evaluation contributes its final converted solver
frame. The frame selector shows the original iteration and job IDs. Plots use
iteration as the series parameter. Failed, running and unevaluated iterations
are omitted; iteration numbers retain their original gaps. If a completed
iteration cannot be converted or its topology elements cannot be matched, opening
or refreshing reports the affected iteration and leaves existing sources intact.

The default cutoff is 0.5. Under Blocks / Element sets, **Retained / <block>**
and **Removed / <block>** are separate, directly selectable datasets. Retained
sets start visible and Removed sets start hidden. Preserved elements and elements
at or above the cutoff are retained, matching the CalculiX preview. Geometry
outside the design domain and auxiliary solver datasets are available separately
and start hidden. The split affects the Postprocessor display datasets.

To change the split on an open source, select the topology source in **Sources /
Results**. In the **Properties** dock, enter the **Retention cutoff** and click
**Apply**. The current frame, camera and Retained/Removed visibility are preserved.
Playback pauses while the updated geometry is published. Apply changes the sets
for all frames already in the source. Use Refresh separately to add newly
completed iterations. The same cutoff change can be journaled::

  post topology threshold 0.65 source "Topology 1 / simple_beam"

If the original results or topology design records cannot be verified, the
command reports an error and keeps the open source at its previous cutoff.

**Topology Density** is the stored element design variable for density
iterations. For level-set iterations, **Topology Level-Set Coefficient** is a
nodal field and **Topology Solid Fraction** is the element field used for the
split. **Topology Applied Stiffness Factor** is the applied solver factor, and
**Topology Preserved** is the preservation flag resolved by the topology run.
Ordinary solver fields remain available on the split datasets.

Use ``post refresh "Topology 1 / simple_beam"`` to include newly completed
iterations for the same case and cutoff. Project restore verifies the contributing
results and topology values through the saved last iteration before publishing a
snapshot. Refresh verifies the previous frames before appending new ones and
preserves set visibility by dataset identity. Open a new source after a rerun
if the earlier results changed.
