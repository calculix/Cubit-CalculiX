GUI ownership, navigation and editor contracts
===============================================

Owners and host boundaries
---------------------------

``CalculiXGui`` is an internal GUI owner; ``CubitSimComp`` is the loader-facing
component. The Framework hosts workspace views. The CalculiX GUI creates its
model tree, properties dock, drawing/topology toolbars, menus, panel manager and
observers, and removes its resources during ``stop()``. Qt parent ownership and
QPointer deletion tracking have distinct roles. Timers and auxiliary windows
must not keep calling SDK objects after workspace/application teardown.

Source: :download:`CalculiXGui.hpp <../../src/CalculiX/GUI/CalculiXGui.hpp>`
and :doc:`framework-stage3`.

Navigation and factory registration
-------------------------------------

``cmdPanelManager`` builds navigation nodes and marker registrations.
``MyCmdWidgetFactory`` creates/cache-controls widgets for those markers. A new
navigation feature needs a matching node marker, registration in each maintained
marker list, widget factory branch, source-list registration and lifecycle
clearing. A marker used only as a parent is not necessarily a widget target.

Model-tree context actions and properties are separate entry paths. They must
select the same feature/method as navigation rather than retain a removed generic
marker. Cached widgets can be hidden; selecting the correct class and operation
mode matters when a tree action supplies the entity ID.

Source: :download:`cmdPanelManager.cpp <../../src/CalculiX/GUI/cmdPanelManager.cpp>`
and :download:`MyCmdWidgetFactory.cpp <../../src/CalculiX/GUI/MyCmdWidgetFactory.cpp>`.

Command submission and confirmation
-------------------------------------

``GuiSendCmd::gui_cmd()`` finds the native command window, invokes its
``commandEntered`` method with a direct Qt connection, and returns whether the
invocation was submitted. It is not the command parser's success result.
Commands can process GUI events while executing. Use existing submission and
reentrancy guards, keep the source selection stable, and confirm Core values
before discarding edits when a form promises confirmation.

Preview paths use the existing silent command API to avoid history/console spam.
Read queries can use interfaces directly; GUI mutation should not bypass the
commands used by journals. Numeric text should be validated and converted to a
locale-independent command value. Display formatting can follow the user's
locale without changing parser syntax. Quoted names must reject unsupported
quotes/line breaks rather than concatenate unsafe command text.

Source: :download:`GuiSendCmd.cpp <../../src/CalculiX/GUI/application/GuiSendCmd.cpp>`.

Trees and properties
----------------------

``ModelTree`` owns feature trees and auxiliary management windows. Feature nodes
carry typed selection roles: public IDs, feature kind and, where needed, topology
case/iteration/job references. ``CalculiXPropertiesDock`` dispatches a selection
to the matching property widget. A tree row number is not a public feature ID.

``PropertyTreeWidget`` supplies identity/value/reference rows and validated name
editing. A reference points to another feature domain; an editable name change
must preserve unrelated settings. Refreshing an active property selection after
model changes prevents stale rows from surviving deletion or method conversion.
Not every displayed property is directly editable.

Independent editors and management windows
-------------------------------------------

Material Management, Steps Management and Jobs Monitor have their own complete
workflows. A command panel is a navigation editor, not a model store. A management
window can share interfaces, command contracts and plain records without
embedding the command panel widgets themselves.

Topology follows this arrangement:

.. code-block:: text

   Navigation Create / Modify
       Density-based -> TopologyDensitySettingsPanel
       Level set     -> TopologyLevelSetSettingsPanel

   Management Settings tab
       Method dropdown -> QStackedWidget
           TopologyManagementDensitySettingsWidget
           TopologyManagementLevelSetSettingsWidget

Each form owns its fields, original values, drafts and command construction.
Management supplies its existing command guard and post-apply callback. Choosing
a Method changes the displayed form; Apply changes the stored case. Historical
iterations retain their captured method. Reset/deletion must clear affected
forms, selection, properties and monitors, while preserving unrelated case drafts
when the operation permits it.

``TopologyTree`` routes Modify settings to the stored method's marker, exposes
method-specific Create actions and opens Management on case/root double-click.
Iteration properties use the captured method for the field/density-change label.
The current Actions panel still has a method-specific discrepancy recorded in
:doc:`review-findings`; documentation does not silently treat it as fixed.

Drawing and toolbars
----------------------

``CoreDraw`` owns command-driven scene state and rendering/selection behavior.
GUI toolbars control it through drawing/topology commands and query its published
state. Scope, symbol size, overlay visibility, workspace activation and native
display clearing have different meanings. A native refresh can clear temporary
SDK drawing sets; the observer coordinates overlay visibility so a preview does
not immediately reappear from stale state.

Topology preview uses selected case, completed iteration and threshold. The
toolbar contains one-line navigation controls and a preview toggle. Menu actions
and workspace toolbar persistence must agree about intended visibility.
Case/iteration dropdown text is presentation; actions must retain IDs as data.
GIF capture must coordinate with preview/playback timers and stay on the host
thread for native screen capture. Encoding can use detached file data.

Source: :download:`CoreDraw.hpp <../../src/CalculiX/Core/drawing/CoreDraw.hpp>`
and :doc:`topology-results`.

Verification boundaries
------------------------

Source inspection can verify marker spelling and callback wiring, but cannot
prove native focus, display clearing, menu lifetime, final application shutdown
or Full HD usability. Keep platform GUI checks separate from parser/model tests.
No screenshots or UI pass claims were generated during the documentation-only
work; see :doc:`topology-verification` and the downloaded GUI checklist.
