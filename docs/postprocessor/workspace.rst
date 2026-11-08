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
   Postprocessor menu: File | View | Results | Help
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
The independent **Postprocessor Mouse** options page assigns Rotate, Zoom or Pan
to each drag button. Defaults are left Rotate, middle Zoom and right Pan. The
wheel zooms.
The default background is white/white, with a bottom-left XYZ triad.
Postprocessor options control colors and triad visibility. Postprocessor Mouse
controls button assignments, sensitivity and reverse zoom independently of Cubit's
settings. F9-F12 and Shift variants
select standard views as described in :doc:`commands`.
``F`` fits and ``Esc`` cancels an unfinished interaction. GUI camera actions
are silent by default; enable **Record camera navigation** in Postprocessor
options to record completed actions. Source/result commands keep their normal
journaling behavior. Touch and native Cubit viewport handlers remain disabled.
There is no timeline or split-view control yet. Postprocessor preferences remain
in Cubit's existing options system.
Additional menu groups will arrive with their actions.

See :doc:`../developer/postprocessor-workspace` for integration details and
:doc:`../developer/postprocessor-verification` for native verification steps.

Navigation frames follow Qt presentation callbacks instead of a fixed 30/60 Hz
preview timer. Only the latest pending input is consumed for each requested frame.
Rendering is idle when nothing changes and paused while hidden. Actual frame rate
depends on geometry cost, Qt composition, the operating system and driver support
for the requested swap interval. It is not a promise of reaching the monitor Hz.

Mesh edge display
-----------------

The shaded surface may be tessellated for nonlinear elements. Its tessellation
edges are hidden. A separate line actor extracts the original element edges
from the published mesh, retaining quadratic midside nodes. Opaque surfaces
occlude internal edges. This changes only the display; result connectivity and
field arrays remain untouched. Both actors follow the representation visibility.

Model reset
-----------

Cubit ``reset`` also resets the Postprocessor session, including when the
workspace is hidden or its GUI has not been started. A module-owned model
observer calls the application reset interface; it submits no additional command.
Reset releases Postprocessor-owned result handles, removes representations and
selection, and restores the default single empty view and camera state.
User options, dock placement and the active workspace are preserved.
Externally owned published result handles remain valid; resetting does not
force other applications to release their data.

Scene and camera/fit revisions advance rather than restart. Old gestures and
fit completions cannot restore pre-reset state. Reset during a provider operation
is rejected with an error, preserving the existing Postprocessor session.
