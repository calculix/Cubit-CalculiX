Topology Management
===================

Open the window by double-clicking **Topology** in the model tree. Double-clicking
a case selects it; double-clicking an iteration opens the Iterations tab at that
result. The window is reused when opened again. It has its own controls and does
not embed the command panels. Create, Modify, Preserved sets, Actions, and Delete
remain independently available in navigation.

Cases and settings
------------------

The left-hand list shows case name, ID, and run state. **New** opens the Settings
tab in create mode with a suggested unused ID. Empty optional fields use core
configuration defaults. **Apply** submits creation or modification through the
existing commands. It confirms the stored settings before clearing edits.
The **Method** dropdown displays the complete Density-based or Level set settings
widget inside the tab. Switching the dropdown alone does not modify the case;
Apply stores the selected method. Each method retains its own draft.
Level-set cases can be configured, but their Run button remains disabled pending
native numerical verification.

Changing cases retains unsaved settings as window-local drafts. **Reload settings**
discards that case's draft and reloads stored values. Drafts are not model data
and are cleared when the model is reset. Background refresh does not overwrite
an editor's draft. **Delete** submits deletion of the selected case and its linked
job/result records; it does not delete the current mesh.

Preserved sets
--------------

Choose Blocks, Nodesets, or Sidesets. Select entries and use Add/Remove, or
double-click an entry to move it. Restore automatic defaults clears explicit
inclusion/exclusion rules. Rules affect subsequent runs. An element can remain
preserved through another effective set or a direct selection. See
:doc:`topology-preservation`.

Run
---

Run starts asynchronously unless **Wait until finished** is checked. Wait and Kill
use the existing topology commands. Starting again resets that case's earlier run
records. The Run tab shows the active job, current iteration and the established
convergence indicators; its job button opens the existing Jobs Monitor.

Results and exports
-------------------

The Iterations tab contains browsing, preview, playback, threshold-based deletion,
physical/adjoint job access. GIF exports all completed iterations.
Alpha wrap exports the retained elements of the displayed iteration as an STL.
The result threshold and displayed iteration are shared selection values within
this window. See :doc:`topology-results` for limitations and interpretation.

Previews use silent commands. Editing, preservation, running, deletion, GIF export,
wrapping use recorded commands. GIF cancellation uses the existing
animation-controller API. Closing the window pauses preview playback and hides
the window; it does not kill a topology run or cancel an active GIF export.

Reset, save and restart behavior
--------------------------------

.. list-table:: Model records, controller state and mesh changes
   :header-rows: 1
   :widths: 28 72

   * - Operation
     - Behavior
   * - Save/load a model
     - Stores supported settings and iteration records. External solver inputs/results remain separate files; saving topology data does not package them all.
   * - Load a case saved while running
     - Restores an interrupted failed state, not an active solver process or controller context.
   * - Run the same case again
     - Clears that case's previous run records and initializes a fresh run with its current settings and preservation selections.
   * - Reset or delete records
     - Clears or invalidates affected GUI selections, monitor data and previews. Historical IDs must not be reused by stale controls.
   * - Delete mesh elements
     - Keeps historical numerical records, but missing elements can prevent later preview, playback, GIF export, wrapping or further projection.

Model reset clears management data, cached topology/CGAL panel selections and
Jobs Monitor. Deleting a job explicitly clears a monitor displaying that job,
including its FRD/DAT pages and plot windows, before the ID can be reused.
Deleting topology iteration jobs or resetting/deleting a case invalidates that
case's displayed management data and preview. Unrelated settings drafts remain.
Notifications are optional GUI callbacks; headless commands need no GUI component.
Jobs Monitor retains its ParaView button. Run controls are positioned below the
convergence and displacement information at the bottom of the Run tab.

Topology toolbar
----------------

**View > CalculiX topology** shows or hides the single-row toolbar. It contains
case selection, First/Previous/Next/Last navigation, completed-iteration
selection, a six-decimal threshold, an icon-only **Topology Preview** toggle, and
a final **CGAL Alpha wrap** button.
Dropdown widths follow their contents with a compact minimum width. Each control
has a tooltip. Open management by double-clicking the model-tree case or iteration.
The displayed iteration is a GUI selection; browsing does not change the stored
core selected iteration. Preview and mesh actions submit the displayed ID
explicitly. The remembered toolbar threshold/visibility preferences are separate
from that model selection and from transient controller state.

The toolbar's Alpha wrap button wraps the selected completed iteration at the
shown threshold using automatic alpha and offset. It writes
``topo_<case>_iteration_<iteration>_wrap.stl`` in the working directory, overwrites
that file if it exists, and imports the STL into Cubit. The command is recorded.
Use the Alpha wrap tab in management to choose other export settings.

With Topology Preview enabled, changing the iteration or threshold updates the
preview through a silent command. Disabling it hides only the topology group.
A successful preview enables CalculiX display. Cubit's native Refresh Display
hides all CalculiX overlays while retaining their settings and selections; a
subsequent preview shows the topology again.

Toolbar visibility and layout survive workspace switches and application
restarts. The threshold is a GUI preference; case and iteration IDs are scoped
to the current model. Model reset clears these selections.
