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

Open **Help > cubit-sim documentation**, or run::

   sim documentation

This opens the installed ``documentation/index.html`` beside the Framework
libraries using the desktop's configured browser. It requires a GUI-capable
session. An accepted request does not guarantee that the browser has finished
loading. A missing or unreadable manual reports its expected path.

The manual needs no server or internet connection for navigation, search,
formulas or packaged downloads. External solver-manual citations remain online
links. See :doc:`../README` for installation and packaging of the manual.
