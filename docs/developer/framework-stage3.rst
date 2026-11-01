Stage 3: Cubit and CalculiX workspaces
====================================

The framework command plugin provides::

  fea workspace cubit
  fea workspace calculix

The header switch submits the same commands through Cubit's command widget.
The selected tab changes only when activation succeeds. Commands require
the GUI component; loading the component with ``withGUI = 0`` still initializes
common CalculiX features without creating workspace widgets.

Ownership and startup
---------------------

``FEAComp`` attaches the workspace host before starting internal components.
This captures the native layout and shared widgets before CalculiX adds its
own hidden docks and menus. After startup, optional ``WorkspaceProviderInterface``
implementations return borrowed workspace views. All widgets and navigation
models remain owned by their providing components.

The initial workspace is Cubit. CalculiX owns a separate NavigationModel, the
existing model tree, an empty ``CalculiXPropertiesDock`` and File/View/CalculiX/
Help menus. The full native Block, Nodeset and Sideset navigation branches are
copied with their icons, labels, tooltips, search tags and dropdown grouping.
An adapter borrows their panel factories using the original native model/index,
including nodes without markers. All native descendants remain available within
CalculiX navigation. Cubit Blocks is a separate top-level branch beside the
original CalculiX Blocks branch. The CalculiX block and modify icons are retained;
its element-type panel is not nested under Cubit Blocks. The shared TaskPanel
changes model/root when selecting a workspace.

CalculiX supplies the entire navigation model to TaskPanel, retaining CCX as
the first navigation level. Cubit's CommandButtonPanel creates three button
levels followed by dropdowns: CCX, entity, operation, then panel choice.
Setting CCX itself as the display root removes a level and incorrectly renders
panel choices as the third button row. Display-type flags do not replace these
fixed button levels. Single-panel operations bind directly to their factory,
including block Modify and contact-pair Utility; they have no additional
panel-choice node. Native Cubit branches retain their copied display settings.
CalculiX requests hideNavigationMode to hide the redundant single-button Mode
group without removing CCX from the navigation path. The host retains Cubit's
original Mode visibility and suppresses Show events for that group while the
workspace requests it hidden. Returning to Cubit restores its original visibility.
An initialNavigationSelection explicitly selects CCX on first entry before hiding
Mode, so its entity buttons are populated without requiring the hidden button to
be clicked. Saved selections take precedence on subsequent workspace switches.

The header retains the test component's icon-prefixed tabs and right-aligned
Workspace caption, but is a separate QWidget directly beneath the window title
and above the native menu. Main-window contents margins reserve its row, with
WA_LayoutOnEntireRect disabled so Qt respects that space. Its height is computed
from the populated tab bar rather than a cached layout size hint. As it is not a
toolbar, workspace restoreState cannot relocate it. The CalculiX icon is loaded
directly from CCX.svg through the existing icon loader. No Qt resource compilation
or menu style adapter is used. Original margins and the layout attribute are
restored on removal.
Native toolbars are not removed/reinserted.
The existing Claro menu-bar object is retained. Theme, font and scaling changes
require verification in the real Cubit GUI.

The CalculiX GUI appends its name and core-provided version through Claro's
set_title API during startup. Cleanup removes only that suffix. Title ownership
belongs to the component GUI lifecycle, not the model tree or workspace switch.

Shared and workspace controls
-----------------------------

The Cubit viewport, command/error/journal widget and toolbars are retained.
The status area and progress bar are not managed as workspace widgets.
In CalculiX, native model/properties and other native side docks are hidden;
the command and navigation docks remain. Power Tools sibling tabs are hidden
when they share a tab container with navigation. Dock visibility actions are
restricted to the active workspace.

The initial CalculiX layout places its model and empty properties dock in a
vertical left split. Bottom corners belong to the side docks, leaving the
shared command dock below the middle viewport. Layout restoration protects
the shared command dock and retains current native toolbar visibility. Toolbar
positions come from Qt's saved layout; native toolbars are never removed and
reinserted by the workspace host.

Settings and cleanup
--------------------

Layouts are GUI preferences in ``QSettings("Cubit-CalculiX", "Framework")``
under ``Workspaces/<id>/Layout``, with layout version 1. Runtime navigation
selections are tracked per workspace. No model data or .cub5 format changes
are introduced. The old model-tree-wide saveState/restoreState calls are removed.

The workspace manager restores Cubit before component-owned widgets and the
CalculiX navigation model are destroyed. Activation validates the host and
registered workspace, snapshots the old state and rolls back failed switches.
Errors use CubitMessage through the command/component entry points.
The host close-event filter restores Cubit before Claro saves its native layout.

Validation boundary
-------------------

Real GUI checks must cover repeated switches, journal
commands, layout restoration, native panel shortcuts, toolbar/progress/viewport
continuity, Power Tools hiding, themes, display scaling and component unload.

Startup crash investigation
---------------------------

The first access violation occurred in Qt's toolbar area layout on the main
window's first show. Restoring workspace state and then removing/reinserting
native toolbars left an invalid layout item in Qt's pending layout state.
The host now leaves toolbar ownership and positioning to QMainWindow and only
reapplies their visibility. Do not reintroduce removeToolBar/addToolBar loops
around restoreState.

A second access violation during process teardown hid the first failure:
WorkspaceManager's destructor attempted saveState after Qt had destroyed the
main-window layout. Cleanup is now explicit, idempotent, and connected to
aboutToQuit; UI access checks application state, GUI thread and layout lifetime.
The destructor does not save or restore widgets.

Set FEA_STARTUP_TRACE to an absolute log filename for optional startup and
workspace lifecycle tracing. Normal errors continue through CubitMessage.
