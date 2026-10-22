Topology result commands
========================

These commands inspect or process stored density iterations. Examples assume
case 1 has a completed iteration 4 and the original participating mesh still
exists. See :doc:`../user/topology-results` for the GUI workflow.

Threshold and iteration selection
---------------------------------

Thresholds must be finite and within [0, 1]. Retain an element when it is
preserved or its density is greater than or equal to the threshold. Only
unpreserved elements strictly below the threshold are removal candidates.
The default threshold for draw, project, and animate is 0.5.

For draw and project, omitting ``iteration`` uses the core's stored selected
iteration. Successful iteration completion updates that selection. The Results
panel's displayed iteration is separate and is passed explicitly in its
commands. There is no separate user-facing selection command documented here;
use explicit positive iteration IDs for reproducible journal operations.
An omitted iteration fails if no valid selection exists.

Draw a preview
--------------

.. code-block:: text

   ccx draw topology <id> [iteration <id>] [threshold <value>]

Draw retained elements in green and removal candidates as red edges. This
changes the display and leaves the mesh unchanged.

.. code-block:: text

   ccx draw topology 1 iteration 4 threshold 0.5

Preview or delete elements
--------------------------

.. code-block:: text

   ccx result project topology <id> [iteration <id>] [threshold <value>] [preview]

With ``preview``, draw the threshold classification without deleting elements.
Without ``preview``, delete the removal candidates. Preserved elements are
excluded from deletion. The case must not be running, and the requested
iteration must be completed and evaluated.

The command validates the whole stored design against the current mesh before
starting deletion. Deletion then runs in batches. There is no restoration point
or rollback: a failed batch can leave earlier batches, and possibly part of the
failed batch, deleted. A display-refresh error can also be reported after the
mesh has changed.

.. code-block:: text

   ccx result project topology 1 iteration 4 threshold 0.5 preview

Run the following separately after reviewing the preview:

.. code-block:: text

   ccx result project topology 1 iteration 4 threshold 0.5

This operation removes elements; it does not project displacements or transfer
the iteration's computed stiffness factors to a new analysis.

Export a GIF
------------

.. code-block:: text

   ccx animate topology <id> [threshold <value>] [fps <value>] [save <filename>] [overwrite]

Export all completed, evaluated iterations with volume data in numerical order
as a looping GIF. The case must not be running. Every frame uses the same
threshold and the current graphics view. Frame capture previews the designs;
the command can leave the final design displayed.

The command starts an asynchronous export and returns after startup. Startup
success does not certify that the GIF was created. Capture runs one frame at a
time on the GUI thread; encoding, publication, and temporary PNG cleanup run on
a worker. The saved-file message is printed only after those stages finish.
A single complex capture can still briefly pause the UI.

Only one topology GIF export can be active. Use **Cancel GIF export** in the
Results panel to request cancellation. Cancellation is checked between captures
and encoded frames; it cannot interrupt one encoder frame already in progress.
An output already published before cancellation is retained. Avoid changing the
mesh, model, camera, or graphics view during capture. Model reset/load requests
cancel the export; starting topology is rejected while export is active.

* ``fps`` defaults to 5 and must be an integer from 1 through 100.
* ``save`` defaults to ``topo_<id>_animation.gif`` in the working directory.
* The filename must end in ``.gif`` and its parent directory must exist.
* An existing output file requires ``overwrite``.

.. code-block:: text

   ccx animate topology 1 threshold 0.5 fps 5 save "beam.gif"
   ccx animate topology 1 threshold 0.5 fps 5 save "beam.gif" overwrite

There is no iteration argument: animation processes the case's completed
iterations rather than the Results panel's currently displayed iteration.

Wrap a retained design
----------------------

.. code-block:: text

   ccx wrap topology <id> iteration <id> threshold <value> [alpha <value>] [offset <value>] file "<filename.stl>" [no_import] [overwrite]

Wrap the retained elements of one completed iteration into an STL. The case
must not be running. The iteration, threshold, and filename are required.
All stored elements are validated against the current mesh, including those
below the threshold. Preserved elements are always included. An empty retained
selection is rejected.

* Omitted ``alpha`` uses the input bounding-box diagonal divided by 40.
* Omitted ``offset`` uses the resolved alpha divided by 30.
* Explicit alpha and offset must be finite and positive, in model length units.
* ``no_import`` skips the automatic STL import into Cubit.
* ``overwrite`` permits replacing an existing output file.

Wrapping uses current Cubit mesh coordinates. Volume input is reduced to its
boundary surface, and higher-order elements use a corner-node approximation.
The source volume elements remain in place.

.. code-block:: text

   ccx wrap topology 1 iteration 4 threshold 0.5 file "beam.stl" no_import
   ccx wrap topology 1 iteration 4 threshold 0.5 alpha 2 offset 0.1 file "beam_fine.stl" no_import

Review and export before deletion
---------------------------------

For an existing result, the following sequence preserves the source mesh:

.. code-block:: text

   ccx result project topology 1 iteration 4 threshold 0.5 preview
   ccx animate topology 1 threshold 0.5 fps 5 save "beam.gif"
   ccx wrap topology 1 iteration 4 threshold 0.5 file "beam.stl" no_import

After reviewing the result, execute the deletion command separately. Deleting
stored elements can prevent further mesh-dependent operations on any iteration
of the case. Numerical iteration records do not reconstruct deleted elements.
