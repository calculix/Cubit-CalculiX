Postprocessor workspace
=======================

The current source adds the Postprocessor workspace shell. Its native GUI and
OpenGL behavior have not been tested yet. The previous Core/command build was
reported successful by the user; that does not verify the new workspace patch.

Open the workspace
------------------

Select **Postprocessor** in the workspace header or enter::

   sim workspace postprocessor

Return with ``sim workspace cubit`` or ``sim workspace calculix``. The header
stays below the title and above the menu. Switching workspaces retains sources,
jobs and the Cubit model. It selects a graphics page rather than sharing a renderer.

Layout and actions
------------------

.. code-block:: text

   Workspace header: Cubit | CalculiX | Postprocessor
   Postprocessor menu: File | Results | Help
   Toolbar: List sources | Close selected source | Fit visible results
   +-------------------+-----------------------------+--------------------+
   | Sources / Results | Own Postprocessor viewport   | Properties         |
   |                   |                             |                    |
   | Opened sources    | Independent mesh view        | Selected name      |
   |                   |                             | Application / job  |
   |                   +-----------------------------+                    |
   |                   | Command / Error / History   |                    |
   +-------------------+-----------------------------+--------------------+
   Cubit status and progress

The left dock lists opened source names. Selecting a source submits the existing
``post select`` command. The right dock shows read-only source information. Sources
opened or closed from the command window also update the panels after command
completion. Programmatic refresh does not submit selection commands.

Mesh sources have a visibility checkbox reflecting the active view. Checking or
unchecking submits ``post show "<name>"`` or ``post hide "<name>"``. It does not
change visibility in other views or unload the source data. Table-only sources
remain selectable without a mesh checkbox.

**List sources** submits ``post list``. **Close selected source** submits a quoted
``post close`` command. **File > Close all sources** submits ``post close all``.
**View > Fit visible results** and the fit toolbar action submit ``post view fit``.
The shared **Help > cubit-sim documentation** uses ``sim documentation``.
Help is the last menu in every workspace. Closing a source does not
delete its files or solver job. The new toolbar uses Cubit's command-window and
cancel and zoom-reset SVG resources, with standard Qt icons as a fallback.

The shared command dock remains below the middle graphics column, with floating,
movement and closing disabled while this workspace is active. Native Cubit
navigation, docks and toolbars are hidden here. Their prior properties are restored
when leaving. The existing Cubit status/progress widgets remain hosted by Cubit.

Familiar Cubit icons do not invoke Cubit's viewport tools. Postprocessor controls
operate on its own camera, selection and display state through Postprocessor
commands. Shared command/journal hosting does not imply shared viewport interaction.

Current limits
--------------

Load job data using the :doc:`commands`, for example ``post open ccx job 12``.
Opening a mesh source creates a visible representation in the active view and
selects the first increment containing a supported nonempty mesh. Single grids
and partitioned collections are drawn as neutral surfaces with edges. The first
visible mesh fits the initially unfitted camera. Additional sources retain an
established camera; use ``post view fit`` to include all visible sources. Hidden
sources retain data, and closure removes their displays. Published result arrays
are not modified. Table-only results create no mesh display.
The viewport has its own render window, renderer and camera. Field coloring is
not implemented yet.
Mouse, keyboard, touch and gesture interactions are blocked until journalable
orbit/pan/zoom operations are implemented. There is no timeline or split-view
control yet. Postprocessor preferences remain in Cubit's existing options system.
Additional menu groups will arrive with their actions.

See :doc:`../developer/postprocessor-workspace` for integration details and
:doc:`../developer/postprocessor-verification` for native verification steps.
