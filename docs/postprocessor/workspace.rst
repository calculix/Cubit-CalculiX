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
   Postprocessor menu: File | View | Results | Tools | Help
   Toolbar: Cubit New/Open/Save | Journal controls | Fit | Node/Element/IP
   Results row: [Source selector / Add / Import] [Field] [Deformation] [Navigation]
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
``post select`` command. The right dock shows source information and the editable Display section. Sources
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
delete its files or solver job. The first toolbar borrows Cubit's native New/Open/Save and journal actions,
including their handlers, shortcuts, enabled states and icons. List and Close
remain menu actions. Fit and picking remain Postprocessor viewport operations.

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

Source catalogue and frame navigation
-------------------------------------

The source tree contains Blocks / Element sets, Node sets, Side sets / Surfaces,
Integration points and Results directly below each source. Empty categories are
omitted; there is no intermediate Mesh item. Each result family (for example DISP,
STRESS or TOSTRAIN) appears once, independent of the number of frames or compatible
mesh datasets. There is no Frames branch. Distinct associations, sets, origins or
component layouts remain separate variants beneath the result family. Unavailable
variants remain visible but disabled in the selected frame. Dataset/tooltips
continue to describe mesh structure rather than result-frame history.

The toolbar uses the same result catalogue. Applying a compatible group colors
all its matching datasets in the active view; an explicitly typed dataset qualifier
continues to restrict coloring to that dataset. Frame changes update availability
without replacing result identities or losing tree expansion/selection.

Result navigation is confined to its compact toolbar row. Its dropdown, first/previous/next/last
buttons and horizontal slider select displayable frames, skipping unavailable mesh
frames without renumbering the underlying sequence. Frame labels show solver step,
increment and time supplied by the provider. Slider positions represent frame
sequence positions, not uniformly spaced physical times.

Slider dragging coalesces pending input and previews absolute frame commands through
silent_cmd. Release submits one normal absolute command when Record result navigation
is enabled; the dropdown and buttons commit immediately. With recording disabled,
GUI frame commits use silent commands too. Source opening/closing remains journaled.
Changes of source, view, result snapshot or external frame state invalidate a pending
drag. Hiding the toolbar cancels an owned drag back to its initial frame without a
journal entry. Reset clears pending slider input and stale navigation state.

Result colors
-------------

The toolbar selects a field and component for the active source/view. Double-click
a tree field to apply it: declared vectors initially use magnitude, other fields
use their first published component. These actions submit explicit source/array
commands for the logical result group. Choosing Solid color disables contours while retaining palette,
range and limit preferences. Geometry is not deformed by result coloring.

Classic rainbow is the factory default palette. Viridis, Blue-white-red, Turbo and Grayscale are
also available, with reversal and continuous or 2..256-band display. The current
palette implementations use Core-owned RGB control points. Current-frame ranges cover valid samples of all compatible selected datasets in
the current frame. All frames uses their combined finite minimum and maximum
across the source's result sequence; its scale stays fixed during animation.
Fixed ranges use user-entered bounds. Symmetric ranges use [-M,+M] for the current
frame.

Set **Default result palette** in **Cubit-Sim - Postprocessor** options to choose
the palette for newly created result displays. Save persists this preference;
Reset to defaults stages Classic rainbow until Save. Existing displays retain
their palette, which can be changed separately using Properties -> Palette -> Apply.
Classic rainbow uses blue-to-cyan-to-green-to-yellow-to-red RGB stops with dark
blue and dark red endpoints.

A field absent from the selected frame produces neutral geometry and an
unavailable legend, retaining the requested field selection for later frames.
The legend identifies the source and component; enabled limit labels show their
thresholds and colors separately from the palette range. Palette, range, bands,
limits and legend visibility are editable in the Display section of Properties
and through commands.

Result navigation toolbar
-------------------------

Three separate movable/floatable toolbars share this row in the order **Field
selection**, **Deformation**, **Result navigation**. Each has its own View-menu
toggle. Navigation is last so its slider can use the remaining width. Framework
restores that default ordering on workspace activation, with a row break before
Field selection. The main toolbar retains source/view actions and :doc:`selection`
controls.

The Deformation toolbar contains:
**Deform**, displacement field, **Scale**, and **Undeformed**. There is no Apply
button. Checkboxes and field selection update the selected source in the active
view immediately through journaled ``post deform`` commands, independently of the
navigation journal option. Programmatic refresh does not submit commands.

The field dropdown defaults to **Automatic displacement**. For CalculiX this uses
``DISP`` on ordinary mesh datasets and the converter's calculated
``IntegrationPointDisplacement`` on integration-point datasets. Each dataset/frame
resolves its own role-marked vector. Explicit selection overrides Automatic;
returning to Automatic retains the previous explicit selection internally.
Missing or ambiguous automatic fields leave that dataset undeformed with a warning.
External vectors without role metadata remain available for explicit selection.

Enable Deform and enter a nonnegative scale. Valid scale input previews after a
250 ms typing pause through silent commands. Enter or leaving the field submits
the final normal command once, including when the preview already displays that
value. Incomplete input such as ``1e-`` keeps the last valid display; finishing
retains and, if changed, journals that last valid value. Scale defaults to 1 and
stays fixed during animation. Deformation itself remains off by default. Coordinates and
displacement must use compatible units; the display uses
``position = original position + scale * displacement``. **Undeformed** adds a
neutral wireframe of the original mesh while deformation is enabled. Turning
Deform off retains the field, scale and overlay preference.

Coloring is independent: stress can color the displaced mesh. Source coordinates,
result arrays and tree data are preserved. Only point-associated fields with
declared XYZ vector components are selectable; derived magnitude columns are never
used as coordinates. Both surfaces and actual element edges move. Fit includes
the displaced mesh and, when enabled, the original overlay.

Missing vectors display the current frame's original mesh with a warning in the
command output. Invalid or masked displacement samples keep original coordinates;
arithmetic overflow is handled the same way. Previous-frame geometry is never
retained as a substitute. Hover the toolbar's **Error** label for command failures.
Rejected changes reload the actual settings. Source, view or snapshot changes
cancel pending scale input; ordinary frame navigation preserves an active scale
edit. Hiding the toolbar cancels an owned preview silently. A net-zero scale edit
restores silently rather than adding a journal entry. Each completed checkbox,
field or scale action is independent.

Result navigation is the last toolbar on the shared results row beneath the
main Postprocessor toolbar. It contains frame buttons, Play/Pause, the frame
selector, slider, FPS and Loop without an expanding dock area. The toolbar can
be moved or floated; View > Result navigation controls visibility. The Framework
reasserts workspace toolbar row grouping on activation and preserves visibility.
Command Line remains in its own full center-column dock.

Animation controls
-------------------

Result navigation includes a Play/Pause button, an FPS spinbox and a Loop
checkbox. Play starts from the current frame, skipping frames without displayable
meshes. Starting from the final frame restarts from the first. Loop is initially
off, so playback stops at the end. Check Loop to repeat the sequence; changing
it during playback applies immediately. Playback does not interpolate physical
time between result frames. FPS is a target rate and may be lower for expensive
frame updates.

Manual stepping or slider dragging pauses animation. Hiding the navigation toolbar
or leaving the workspace pauses it too. FPS/Loop settings remain associated with
that source/view; another representation starts with its own Core defaults.

Display Properties editor
--------------------------

The right Properties dock contains source information and a scrollable Display
editor for the selected source's representation in the active view. Field and
component are read-only here; use the toolbar or result tree to select them.

Edit palette, reversal, continuous/banded contours, band count, range mode, fixed
bounds, independent minimum/maximum limits and their colors, and legend visibility.
Numeric inputs accept scientific notation using a decimal point. Fixed bounds
are editable only in Fixed mode; their disabled values are retained fixed-range
settings, not computed extrema. Choose All frames to keep colors and the legend
on a common scale throughout playback. Each limit's value/color
controls are enabled by its checkbox. Disabled limits retain their saved values.

Changes are staged until Apply. A valid draft is applied through existing
source-qualified color/legend commands. Animation and frame updates preserve
unsaved edits while the representation, published result and committed coloring
stay the same. A source/view/result replacement or an external coloring change
reloads the committed settings. No selected representation disables the editor.

Apply validates the complete draft before submitting anything. Limit updates
may temporarily disable/configure/re-enable endpoints so intermediate command
states remain valid. If submission or a handler fails, remaining commands stop
and the panel reloads the state that actually committed, with an inline error.
Successful earlier commands remain applied; Apply is not a multi-command undo
transaction. Closing a color dialog after its owning context changes does not
apply that color to the newly selected representation.

All-frame legend range
-----------------------

In Properties > Range, choose All frames and Apply, or run
``post color range allframes source "first_run"``. The legend then spans the
selected field/component's valid minimum and maximum over every compatible
dataset and frame. Missing, masked and nonfinite values are excluded. Limit
highlighting does not change this range. Values can therefore change color
during playback without the legend being rescaled each frame.

The first calculation scans the selected source; its numeric result is cached
for that immutable snapshot and selection. A refreshed/replaced source computes
a new range. No valid samples means an unavailable range, not an invented zero
interval. Use Fixed to set explicit legend bounds manually.

Dataset visibility and isolation
--------------------------------

Mesh dataset rows have checkboxes and a context menu with Show, Hide, Show only
and Show all datasets. The source checkbox remains a master switch; toggling it
preserves child dataset choices. A dataset checkbox represents its stored choice,
so it can remain checked while its parent source is hidden. Show on a dataset,
Show only and Show all datasets also enable that parent source.

Show only isolates datasets within the selected source, leaving other sources
unchanged. Visibility belongs to the active view and survives frame changes and
animation. Datasets absent in the current frame remain listed with an availability
note and can still be configured for later frames. A source refresh clears old
dataset overrides because a new snapshot may reuse IDs for different datasets.
Reset and source closure remove their representation settings normally.

The GUI journals unique published dataset names; duplicate or unrepresentable
names use numeric IDs. Unnamed datasets display Dataset N and use IDs. Tooltips
include the actual ID, independent of the dataset's position in the tree.
Independent single-grid sources without dataset metadata expose dataset 0.

Fit excludes hidden datasets and their undeformed overlays. Current-frame and
symmetric automatic ranges use only visible compatible datasets. All frames
combines their valid extrema across frames and remains fixed during playback;
changing visibility can change that range. Fixed bounds and limit thresholds
remain unchanged. Legends disappear when their scoped datasets have no visible
geometry. Hiding keeps source arrays and cached actor geometry in memory.


Category visibility and project state
-------------------------------------

Blocks, node sets, side sets and integration-point categories have checkboxes.
A partial checkbox means some children are hidden. Toggling a category updates
all its datasets in one command and one scene publication. Source checkboxes
remain master switches and retain individual child choices.

Native project save/open includes Postprocessor references and display settings,
without a second copy of CalculiX result arrays. Missing or changed sources show
an unavailable label and reason, retaining their saved catalogue and settings.
Saved cameras are preserved and playback restores paused. CalculiX retained
results must be saved by its workspace for those references to resolve.


Adding and importing sources
----------------------------

The second toolbar row begins with a job selector, an Add (+) button and an Import
folder button. Jobs are grouped by workspace and identified by name and ID.
Choosing a job only changes the selector; Add submits ``post open ccx job N``.
The catalogue refreshes when the workspace or dropdown opens. Catalogue reads do
not load or convert result arrays. Running/unavailable jobs are disabled with a
reason. Source controls have bounded width and their toolbar can be moved.

Import accepts FRD, DAT, VTU, VTPC and VTT. CalculiX import displays optional DAT
and INP companions, suggesting existing same-basename files for confirmation.
Only explicitly selected paths enter the journal command. Imported CalculiX data
uses detached readers and the existing converter, without creating a job or
querying the current Cubit mesh. INP establishes sets and element formulations;
FRD coordinates remain authoritative after a compatible INP match. Without an
INP, DAT-only spatial records appear as tables containing their original IDs.
Exact formulations may require an INP to locate integration-point output.

VTU/VTPC imports retain original arrays, identifiers and collection structure.
Displacement semantics are not inferred merely from a three-component array.
VTT opens as a table. External sources remain references in project storage;
VTPC dependencies and INP includes participate in integrity checks.

Right-clicking empty space offers Add job results and Import result files.
Right-clicking a source additionally offers Show, Hide, Show only this source,
Refresh, Remove and Remove all. Mesh visibility actions are disabled for
unavailable/table-only sources. Remove closes a source without deleting files or
jobs. Remove all asks for confirmation. Dataset and field menus are retained.
All mutations use commands; listing jobs and opening menus do not journal.


Undeformed integration-point reference
--------------------------------------

When Deform and Undeformed are enabled, integration-point datasets also draw the
original FE edges of their parent elements. Parent lookup uses integral original
element IDs in the same source/frame, independently of the ordinary block's
visibility. Original coordinates and higher-order cell topology are retained.
The outline follows its IP dataset's visibility, uses the existing reference
color and cannot be picked. Repeated IP parents and identical overlapping block
cells are deduplicated. Conflicting or missing parents are omitted with a display
warning; no outline is guessed from a set name or reconstructed from IP positions.


New-view defaults and incomplete external DAT links
----------------------------------------------------

New and reset views default to node picking. New result representations use the
All frames color range, keeping the legend scale fixed while navigating frames.
Projects retain their explicitly saved pick mode and range mode.

External FRD/DAT imports preserve each DAT block as a table when its node/element
IDs cannot be located in the supplied mesh or its IP formulation is unknown.
Other compatible DAT blocks and the FRD mesh/fields remain spatially available.
Original IDs, component columns, set, step, increment and time are retained;
logical fallback table identities remain stable across frames. A matching INP
can supply missing reference nodes and exact formulations. Invalid record schemas
and incompatible supplied INP meshes still fail; this fallback does not invent
coordinates or infer integration rules from FRD geometry type alone.


Initial source visibility and result coloring
---------------------------------------------

A newly opened mesh source shows only Blocks / Element sets when that category is
present in its initial mesh frame. Node sets, side sets and integration points
start hidden. If blocks are absent, the first present mesh category in catalogue
and tree order is shown instead. All datasets in that category are enabled.

The first available result group in tree order with valid samples on those
visible datasets is selected automatically. Declared vectors use Magnitude;
other fields use their first component. The All frames range default is retained.
If no compatible valid field exists, the mesh keeps its solid display color.
These defaults are committed with ``post open``; they submit no extra commands.

Results and its result-variant branches expand when first created, and the active
coloring field is selected in the tree. Later refreshes preserve manual branch
expansion/collapse choices. Reopening an already retained source does not reset
its coloring or visibility. Project restoration keeps saved settings.
