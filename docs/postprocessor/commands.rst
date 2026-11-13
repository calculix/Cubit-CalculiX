Postprocessor commands
======================

Deformation
-----------

Configure a source's representation in the active view::

   post deform field auto
   post deform scale 10
   post deform on
   post deform overlay on
   post deform overlay off
   post deform off
   post deform field "DISP"
   post deform field "IntegrationPointDisplacement" dataset 2

Every form accepts ``source "name"``; omission selects the active source. Explicit
field selection also accepts ``dataset N`` (zero-based, as with result coloring).
The field argument accepts a published name or exact array name. Ambiguous
variants require an exact array name and, if necessary, dataset qualification.
Without a dataset qualifier, compatible datasets in the logical group deform
together. Nonselected datasets keep their original coordinates.

Automatic displacement is the default selection. It resolves one compatible field
marked as displacement by the provider, independently for each dataset/frame.
CalculiX marks ordinary ``DISP`` and calculated ``IntegrationPointDisplacement``;
the integration-point calculation remains in its converter. Vector shape and field
names alone never establish automatic displacement semantics. External data without
role metadata can use explicit field selection.

``field auto`` restores automatic mode and retains the last explicit selection.
It has no dataset qualifier. ``field "name"`` switches to explicit mode and may
target one dataset. Field selection retains the current enabled state. Enabling
requires at least one mesh dataset with an available, unambiguous displacement
field; other datasets may fall back to reference coordinates with warnings.
``off`` retains all settings. Scale defaults to 1 and must be finite
and nonnegative. Scale 0 shows reference coordinates. No unit conversion is
performed. Overlay defaults to off and only appears while deformation is enabled.
Coloring, palette ranges and threshold limits remain independent.

Toolbar checkboxes and field selection submit these commands immediately. Scale
input uses debounced silent previews and one normal journaled command on Enter or
focus loss. Duplicate completion signals and programmatic refresh add no commands.
Incomplete numeric input preserves the last valid display. Pending scale input
is discarded if its source, view or snapshot changes. Completed deformation
changes follow Cubit's normal journaling path, independently of the navigation
journal option.
Unavailable vectors or invalid samples during frame changes use reference
coordinates with a warning, retaining the requested settings for later frames.

Status and scope
----------------

The following command handlers are implemented in source. Native parser behavior,
job loading and journal replay remain unverified until building and testing are
authorized. These commands manage loaded source data. Use
``sim workspace postprocessor`` to enter the :doc:`workspace`; its viewport is
independent and displays supported mesh sources with a neutral surface and edges.

Open job results
----------------

.. code-block:: text

   post open <provider> job <job_id> [name "<source name>"]

Example with an existing CalculiX job:

.. code-block:: text

   post open ccx job 12
   post open ccx job 13 name "Variant B"

The provider prefix is an unquoted string, resolved through Framework's provider
registry. ``ccx`` currently resolves to the CalculiX provider; additional providers
can register their own prefixes without changing this command's syntax. The job
ID must be positive and identify a job in that provider's current application
context. Unknown providers, missing/running jobs and conversion failures fail
the command and are reported through Cubit's error mechanism.

The job name becomes the source name unless a quoted override is supplied. Names
must contain a non-whitespace character and be unique, with case-sensitive matching.
Use the override to open a second independently retained snapshot of the same job.
The source becomes active and successful opening executes
``sim workspace postprocessor``. Existing sources remain loaded. An explicit duplicate
name is rejected before loading; a duplicate suggested job name can be detected
after loading but cannot replace existing Postprocessor data.

The provider returns an immutable result snapshot. CalculiX reuses its completed
in-memory snapshot when available; otherwise it loads and converts the job files.
Opening the same retained snapshot without a name selects its existing source
and makes its representation visible in the active view. A distinct explicit
name requests another source. A newer run does not silently replace an old one.
Opening failure does not switch workspace; explicit refresh reloads result files.
The current CalculiX snapshot token identifies a conversion,
not a durable solver run. Running jobs and active result conversions are rejected.

Show and hide sources
---------------------

.. code-block:: text

   post show "<source name>"
   post hide "<source name>"

These commands affect only the active view. Showing creates its source
representation if needed; repeated show/hide requests are idempotent. Hiding
retains the input data and does not change source selection, an established
camera, or another view's visibility. Unknown names fail. Table-only sources
cannot be shown as meshes. Tree checkboxes submit these same commands and restore
authoritative Core state after failure.

Opening a mesh source automatically creates a visible representation in the
active view, using its first increment containing a supported nonempty mesh.
Each representation stores its own increment index for future time comparison
between views. There is no time-navigation command yet.

Fit visible meshes
------------------

.. code-block:: text

   post view fit

Requests fitting all visible mesh bounds in the active view. With no visible
mesh it succeeds without changing camera or fit state. The Fit button and F key
skip command dispatch in that case, producing no output or journal entry.
The first mesh in an initially unfitted view requests fitting
automatically; later openings retain an established camera. The request preserves
the current orientation/projection as its seed. The GUI completes it when the
viewport is visible and has a valid OpenGL context, using the current viewport
aspect ratio. Its resulting camera is validated and retained in Core.

The command records the fit intent, not exact camera coordinates across different
window sizes. A headless or hidden view retains the request until GUI completion.
Mouse navigation uses the camera commands below.

Camera navigation
-----------------

.. code-block:: text

   post view rotate <yaw_degrees> <pitch_degrees>
   post view pan <horizontal> <vertical>
   post view zoom <factor>
   post view camera position <x> <y> <z> focal <x> <y> <z> up <x> <y> <z> projection {parallel|perspective} angle <degrees> scale <half-height>

Rotation orbits the focal point. Positive angles follow the right-hand rule:
yaw around orthonormal view-up, then pitch around the resulting right vector.
Pan moves position and focal point in scene units along screen-right/up. Zoom
requires a positive finite factor; values greater than one zoom in. Perspective
zoom moves the camera toward the focal point; parallel zoom divides its half-span.
Relative operations require an established camera. The absolute command supplies
all values and can establish one directly. Invalid values preserve existing state.

Drag assignments are configurable in **Postprocessor Mouse**. Defaults are left
Rotate, middle Zoom and right Pan, independently of native Cubit mouse preferences.
The wheel always zooms. A drag assigned to Rotate and started outside the central
navigation circle spins about the view
axis; a drag started inside orbits the focal point. The mode stays fixed until
release. Mouse sensitivity and reverse zoom are independent Postprocessor options.
``F`` fits visible meshes and ``Esc`` cancels the current gesture. F9 selects
Front (Shift: Back), F10 Top (Shift: Bottom), F11 Right (Shift: Left), F12 Isometric
(Shift: Orthographic projection). These use absolute camera commands and the same
camera journal policy. Standard views retain focal point and distance. Coordinates
use Z up: Front from -Y, Right from +X, Top from +Z; Isometric from +X,+Y,+Z.
Orthographic conversion preserves the apparent vertical span at the focal plane.
Native Cubit orientation parity still needs manual verification. F1-F4 have no
assigned operation in the supplied hotkey reference. F5/Ctrl+R redraw and F6-F8
saved views remain reserved pending agreement on their commands. Selection, mesh
slicing, legend, screenshots and model saving require their corresponding tools.
Touch and native Cubit/VTK interactor tools remain disabled. Camera previews are
silent absolute commands. Completed gestures produce at most one normal command
when **Record camera navigation** is enabled; the default is off.

List sources
------------

.. code-block:: text

   post list

Lists exact source names and their origin. ``*`` marks the active source. The empty
case reports ``No Postprocessor sources are open.`` and succeeds. Names, rather
than internal IDs, are used in subsequent commands. Listing order follows the
internal source-ID map; it is not a promised creation-time or alphabetical order.

Select a source
---------------

.. code-block:: text

   post select "<source name>"

Selects an existing source by its exact, case-sensitive name and clears stale
active display context. It changes no view, camera or source data. Unknown names
fail without changing selection. Quotes are required, including for single words.

Close sources
-------------

.. code-block:: text

   post close "<source name>"
   post close all

Closing removes the named source and its direct displays from the session. Closing
the active source clears selection. ``post close all`` clears every source and
display, retaining the view/layout definitions. Source files and solver jobs are
not deleted. Caller-owned result handles can remain valid after closure.

A source literally named ``all`` is addressed with quotes:

.. code-block:: text

   post close "all"

The unquoted ``all`` is the bulk-close keyword. Quoted names, spaces and this
keyword distinction are explicitly included in the native verification checklist.

Journaling and replay
---------------------

These are native ``CubitCommand`` handlers registered through the existing
cubit-sim command plugin. Commands entered through Cubit's normal command path
participate in its normal history/journal behavior. No separate journal writer
is introduced. Source-tree actions always submit normal commands. GUI camera
previews execute through ``silent_cmd``; the camera option controls completed GUI
rotate/pan/zoom/fit actions only. Commands typed by the user retain Cubit's normal
journal behavior regardless of that option. Opening, closing, selecting, visibility
and options commands always retain their normal command path.

Replay requires the referenced application job IDs and result files to exist.
The current syntax opens current files, so replay is not a promise of historical
numerical reproducibility after files change. Explicit names make subsequent
selection/closure independent of changed job display names. A duplicate name
still fails; commands never silently replace a loaded source.

Apply user options
------------------

.. code-block:: text

   post useroptions icons "Icons/"
   post useroptions palette rainbow
   post useroptions journal camera on
   post useroptions journal camera off
   post useroptions diagnostics camera on
   post useroptions diagnostics camera off
   post useroptions background top 1 1 1
   post useroptions background bottom 1 1 1
   post useroptions triad on
   post useroptions mouse left rotate
   post useroptions mouse middle zoom
   post useroptions mouse right pan
   post useroptions mouse sensitivity 1
   post useroptions mouse reversezoom off
   post useroptions list

The options dialog submits readable commands for changed settings. The camera
option takes effect immediately after saving; it does not change Cubit's global
journal setting. Core validates and persists options under ``postprocessor`` in the
shared preferences file. Icon-directory changes take effect after restarting
Cubit. Invalid values and failed saves leave the preference and active icons
path unchanged. ``list`` prints the saved setting and any pending restart notice.
Preferences are independent of model data. Display defaults are white at both
ends of the background and a visible bottom-left XYZ triad. Different top/bottom
colors enable a vertical gradient. RGB components must be finite and in [0,1];
one command changes the whole color atomically. Mouse button commands configure
only dragging and accept ``rotate``, ``zoom`` or ``pan``. Multiple buttons may
have the same action. Unapplied options edits are discarded when reopened.
Bindings, sensitivity and reverse zoom are captured when a gesture starts, so
saved changes apply to the next gesture. A click below the system drag threshold
does not execute navigation. Selection and context-menu click tools remain separate. The triad does not participate
in model fitting or selection. Display changes apply after saving without a
restart. Sensitivity must be in [0.01,100]; reverse zoom defaults off.

Lifecycle and deferred commands
-------------------------------

The module owns one application instance per initialized Framework lifetime.
Application state is released at module shutdown. Command objects retain weak
application handles; stale commands fail rather than access destroyed state.
Source refresh and independent result-file opening are registered. Cubit
geometry drawing and additional analysis commands remain separate features.

See :doc:`../developer/postprocessor` for source files and method contracts.

Camera timing diagnostics
-------------------------

``post useroptions diagnostics camera on`` enables diagnostics for the next mouse
drag or wheel burst. It defaults off and is independent of camera journaling.
After releasing the mouse (or ending the wheel burst), one multiline summary is
printed when all associated frames have been presented or explicitly abandoned.
No per-frame console output or files are written. Disable it with
``post useroptions diagnostics camera off``. The setting is saved in preferences;
changing it during a gesture applies to the next gesture.

The summary identifies the view and gesture, input/coalescing counts, rendered
and presented frame counts, unpresented frames, unrendered/superseded requests and
dropped diagnostic gestures and unpaired dispatches. Each stage reports its sample count, average and
maximum milliseconds. A stage with n=0 has no samples, not a measured zero cost.

* Input wait: latest received input to preview consumption; it excludes delay
  before Qt delivers the event. Older coalesced inputs are counted separately.
* Formatting: absolute camera command text construction.
* Command dispatch: the complete synchronous callback, including Cubit parsing,
  command handling and any host work it performs. It is not parser-only time.
* Camera handler: entry to exit of PostViewCameraCommand::execute, including
  argument retrieval, the Core call and any error handling.
* Core camera update: the application-interface set_camera call inside that handler.
* Dispatch outside handler: total dispatch minus the matched handler duration
  for that same invocation. It includes string conversion/Framework dispatch,
  Cubit parsing and surrounding callbacks; it is not exclusively parser time.
* Handler outside Core: matched handler duration minus its matched Core duration.
  It includes argument retrieval and surrounding application/error handling.
* Scene sync: the scene synchronization performed for that rendered frame.
* Triad: orientation-overlay update.
* Render wall time: elapsed CPU time inside VTK Render, including driver blocking;
  this is not a GPU execution timer.
* Presentation wait: Render return to the Qt presentation notification, including
  any intervening GUI work. It is not a direct monitor/photon latency measurement.
* Frame interval: consecutive frame starts within that gesture; it also includes
  voluntary pauses in mouse movement, so compare while dragging continuously.

This patch measures the remaining lag; it does not claim to fix its cause. After
building, enable diagnostics, perform several continuous rotations, then compare
command, Render and presentation maxima in the summaries.

The handler/Core split is paired per dispatch. It does not subtract maxima from
different commands. Missing handlers (for example rejection before execute),
multiple handlers in one dispatch or inconsistent boundaries increment unpaired
dispatches; valid total time remains available, but derived samples are omitted.
Typed commands outside an instrumented GUI gesture do not arm this probe. The
existing diagnostics command controls all measurements; no new syntax is needed.

Result frame selection
----------------------

Select a result frame in the active Postprocessor view::

   post frame 3
   post frame 3 source "first_run"

The short form uses the active source. Frame numbers are positive, 1-based
positions in the loaded result sequence, not solver increment identifiers.
For example, frame 3 may contain step 2, increment 1. The command accepts integer
frame numbers up to the native command limit (2147483647).

Selection is stored on the source representation in the active view. It does not
change another source or view, visibility, or an established camera. Unavailable
frames and frames without displayable mesh fail without changing state. Selecting
the current frame again does not advance the display revision.

Result coloring and limits
--------------------------

Commands apply to the active source and view, unless an explicit source is given::

   post color field "DISP" component magnitude source "first_run"
   post color field "STRESS" component "SXX" dataset 0 source "first_run"
   post color solid
   post color palette viridis
   post color palette bluewhitered
   post color palette turbo
   post color palette rainbow
   post color palette grayscale
   post color reverse on
   post color reverse off
   post color bands 12
   post color bands continuous
   post color range auto
   post color range allframes
   post color range symmetric
   post color range -100 100
   post legend on
   post legend off

Component strings must match the field metadata. Magnitude is available only
for fields whose provider explicitly declares their three physical vector
components. Tensor invariants are not calculated by this feature. Dataset IDs
are zero-based catalogue IDs. Ambiguous field names must use the exact published
array name and, where needed, the dataset qualifier. Table fields are excluded.

Independent lower/upper limit colors are configured using normalized RGB::

   post color limit minimum 0 color 0 0 1 source "first_run"
   post color limit maximum 250 color 1 0 1 source "first_run"
   post color limit minimum off source "first_run"
   post color limit maximum off source "first_run"
   post color limit maximum on source "first_run"

A sample strictly below the minimum or above the maximum receives its limit
color. Equality retains the palette color. Off preserves the threshold and RGB
color; on re-enables that configuration. Both limits default to off. When both
are enabled, minimum must be strictly less than maximum. Limits survive frame
changes and do not alter the palette's numerical range.

Thresholds use the selected result's units. A threshold of 250 is not assumed
to mean MPa: select the correct component and use the units of the model/results.
Missing or invalid values are gray, independently of either limit. The legend
includes the enabled thresholds with their special colors.

Grouped result selection and navigation recording
------------------------------------------------

Without a dataset qualifier, color-field selection groups compatible occurrences
of the same result across datasets. Origins, sets, associations, component layouts
and vector semantics must agree; incompatible variants remain ambiguous and require
a qualified array name or explicit dataset. The same group persists across frames.

GUI navigation recording is controlled independently from camera recording::

   post useroptions journal navigation on
   post useroptions journal navigation off

This option defaults on and controls slider release, frame dropdown and frame
buttons. Slider drag previews are never journaled. Typed frame commands still use
Cubit's normal journal policy. Source opening/closing and coloring operations
remain on the normal command path.

Result animation
----------------

Playback controls operate on the active source/view, or an explicit source::

   post animation play
   post animation pause
   post animation fps 10
   post animation loop on
   post animation loop off
   post animation play source "first_run"

FPS is an integer from 1 to 120, default 10. Loop defaults off. FPS and loop
are independent settings on each representation. Play requires at least two
displayable frames. Play at the final displayable frame restarts at the first;
otherwise it continues from the selected frame. With Loop off, playback pauses
at the final frame. With Loop on, it wraps to the first frame. FPS and loop
changes take effect while playing.

Playback is presented by the visible Postprocessor navigation toolbar. The Core
stores settings and a transient playback request, not a background renderer.
Loading/refreshing another source, reset, closing results and leaving the workspace
stop the corresponding GUI playback. Playback requests are not persistent state
that should automatically resume when reopening a saved session.

Individual animation frames use silent frame commands. GUI Play/Pause and its
final absolute frame follow journal/navigation. FPS and Loop changes use normal
configuration commands. Typed commands always follow Cubit's normal journal
policy. Journals capture playback intent and the final frame rather than every
timer tick; the exact wall-clock playback sequence is not reproduced by replay.

All-frame range mode
--------------------

``post color range allframes [source "name"]`` computes a shared legend/color
range across all frames for the selected field and component. All matching
compatible datasets are included unless the field selection has an explicit
dataset scope. The source's units are retained. Validity masks and finite-value
checks match ordinary field display. No limit threshold clips the calculated
minimum/maximum.

``post color range auto`` retains current-frame scaling. Fixed numeric bounds
and current-frame symmetric scaling remain available.

Dataset visibility
------------------

Select a mesh dataset by zero-based ID or quoted published name::

   post show dataset 2
   post hide dataset "Mesh"
   post show only dataset "Integration points — ELSET_A" source "first_run"
   post show dataset all source "first_run"

All forms accept optional ``source "name"``; omission uses the active source.
Commands affect only the active view. Names are matched exactly within the source;
unknown or ambiguous names fail without changing visibility. Use the numeric ID
to disambiguate duplicate names. Quoted ``"all"`` is a literal dataset name;
unquoted ``all`` in the Show form restores all datasets in the source.

Show enables the selected dataset and its parent source. Hide adds that dataset
to the representation's hidden set without changing the parent master switch.
Show only enables the parent and hides every other mesh dataset in that source.
It does not hide other sources. Show all enables the parent and clears its hidden
set. Existing ``post show "source"`` and ``post hide "source"`` retain dataset
choices. GUI checkbox/context-menu actions use these normal journaled commands.
No result arrays are unloaded, and no camera movement is requested automatically.

Selection and probe commands
----------------------------

Use ``post pick mode node/element/integrationpoint`` to choose the next click kind.
Select by original identity, with dataset ID or quoted name and optional source::

   post pick node 42 dataset "Mesh"
   post pick element 120 dataset 0 source "first_run"
   post pick integrationpoint 3 element 120 dataset "Integration points"
   post pick clear
   post probe

The same command path is used by GUI clicks and controls. Large decimal entity
IDs may be quoted for exact 64-bit transport. See :doc:`selection` for readout
semantics, masks, frame changes, supported identities and precision limits.


Dataset category visibility
----------------------------

Toggle all datasets in a category atomically in the active view::

   post show blocks source "first_run"
   post hide nodesets source "first_run"
   post show sidesets
   post hide integrationpoints

Both ``show`` and ``hide`` accept each category. The optional source qualifier
uses the active source when omitted. Show also enables the parent source; Hide
preserves the master source switch. Empty categories report that no matching
datasets exist. The category tree checkbox submits one of these commands, rather
than a command for every child.


External files and source refresh
---------------------------------

Open external sources without creating a solver job::

   post open ccx file "beam.frd" dat "beam.dat" input "beam.inp"
   post open ccx file "beam.dat"
   post open file "mesh.vtu"
   post open file "results.vtpc"
   post open file "forces.vtt"

``dat`` and ``input`` are optional companions for an FRD source. A DAT primary
accepts an optional ``input``. Every opening form accepts ``name "source"``;
otherwise the file basename is used. Names must be unique. Imports do not
implicitly load same-basename companions; the GUI suggests paths for confirmation.

The external CalculiX provider uses the existing FRD/DAT readers and converter.
It never links implicitly to the current Cubit mesh. A supplied INP is checked
against FRD connectivity and coordinates. DAT without mesh information remains
available as tables; it does not acquire fabricated spatial coordinates.

Source isolation and explicit reload::

   post show only "beam"
   post refresh "beam"

Show only changes source visibility in the active view in one Core publication.
It preserves dataset choices and camera pose. Refresh explicitly accepts newer
file contents, retaining the previous snapshot on failure. An ordinary project
restore instead requires the saved result identity. Refresh can invalidate old
dataset overrides, as documented under dataset visibility.

Source removal uses the existing ``post close "name"`` and ``post close all``
commands. GUI menus label these operations Remove; solver jobs and external files
are retained.

Result labels
-------------

Label creation, renaming, visibility, removal, dragging and extrema controls use
``post label`` / ``post labels`` commands. See :doc:`annotations` for complete
syntax, scope resolution and journal behavior. The toolbar and viewport context
menu use these same handlers; drag previews use ``silent_cmd`` and release
records one final position command.

Plotting
--------

The post plot commands create/manage entity histories and numeric native-table
curves. Pane switching, axis previews, final ranges and CSV export use the same
handlers as GUI actions. See :doc:`plots` for syntax and numerical contracts.

PNG captures
------------

See :doc:`capture` for ``post capture view``, ``post capture plot`` and
``report image add postprocessor`` command options and snapshot guarantees.
