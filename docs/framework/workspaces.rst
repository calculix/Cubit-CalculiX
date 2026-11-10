Workspaces and documentation
============================

The Cubit workspace provides native modeling tools. The CalculiX workspace
provides its model tree, command panels, toolbars and management windows.
The Postprocessor workspace supplies its own source tree, properties dock and
independent empty VTK viewport; see :doc:`../postprocessor/workspace`.
Switch with the workspace header or commands::

   sim workspace cubit
   sim workspace calculix
   sim workspace postprocessor

Switching retains the Cubit model and original viewport; Postprocessor uses a
separate graphics page and scene. Dock layouts and tree selections
are remembered separately for each workspace. Switching does not start or stop
an analysis.

Workspace toolbars form one ordered group. In CalculiX, its drawing and topology
toolbars share a row below Cubit's native toolbars. Postprocessor uses its own
top row while native toolbars are hidden. Framework reapplies this grouping after
restoring a workspace layout, so older saved layouts cannot merge CalculiX
actions into Cubit's row. Individual toolbar visibility preferences are retained.

Open **Help > cubit-sim documentation**, or run::

   sim documentation

Cubit's native Tools menu is shared by every workspace and sits immediately
before Help. Its existing actions, including Options, retain Cubit's handlers.
CalculiX no longer creates a separate application menu containing only Options.

There is one active Help menu, always last. Cubit retains its native Help entries
with the cubit-sim documentation action added. CalculiX and Postprocessor use the
shared Framework Help menu. Workspace modules do not create a second Help menu.
The current documentation action opens the shared manual homepage; workspace-
specific entry pages and contextual help destinations are not implemented yet.

Cubit's native File menu also remains available in every workspace. Ctrl+S uses
the existing native Save action: an unnamed project opens Cubit's Save As dialog,
and a named project saves to its current .cub filename. This applies while focus
is in the main window's viewport, report preview, tree or text fields. Native
modal dialogs retain their own keyboard handling. Cancelling Save As writes
nothing; the Framework does not install competing workspace Save shortcuts.

Explicit project saves and loads report completed state operations once per
participating workspace, including CalculiX, Postprocessor and Report, followed
by Framework metadata status. A workspace failure names that workspace and does
not produce a success message for it. Native Cubit success does not imply that
all additional workspace data was saved. Temporary/autosave callbacks retain
their existing handling and are not reported as explicit project saves.

This opens the installed ``documentation/index.html`` beside the Framework
libraries using the desktop's configured browser. It requires a GUI-capable
session. An accepted request does not guarantee that the browser has finished
loading. A missing or unreadable manual reports its expected path.

The manual needs no server or internet connection for navigation, search,
formulas or packaged downloads. External solver-manual citations remain online
links. See :doc:`../README` for installation and packaging of the manual.

A workspace may declare a graphicsBottomDock and placement revision. Existing
saved layouts are restored before a one-time revision migration places that
dock between graphics and the shared command dock. The migration preserves other
docks and saves its new revision only after successful activation. User placement
is retained on subsequent visits.

The Report workspace follows Postprocessor and is activated with
``sim workspace report``. See :doc:`../report/workspace` for generation, Framework
HDF5 profiles, project persistence, preview and PDF/ODT export.
