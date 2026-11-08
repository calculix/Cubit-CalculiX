Postprocessor commands
======================

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

Requests fitting all visible mesh bounds in the active view. It fails without
visible mesh data. The first mesh in an initially unfitted view requests fitting
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
Touch and native Cubit/VTK interactor tools remain disabled. Camera previews update Core directly through the application interface. Completed gestures produce at most one normal command
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
previews bypass text command dispatch through the application interface; the camera option controls completed GUI
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
No ``post`` refresh/file/draw command is registered in this stage. Independent
VTU/VTPC/VTT files, FRD/DAT file opening, Cubit geometry,
field coloring and time navigation require their later implementations.

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

Paired camera command measurements
----------------------------------

Commands/views/CameraCommandTiming is a synchronous, thread-local probe shared
by CameraInteraction's dispatch boundary and PostViewCameraCommand::execute.
It depends only on the standard library; command handlers do not include GUI
recorder classes and Core retains no profiler hooks or GUI objects. Each dispatch
scope masks the previous scope, including when disabled, and restores it on finish
or unwinding. Guards are noncopyable and must end before their enclosing dispatch.
There is no per-call output, heap storage, thread synchronization or global journal
change. Disabled scopes perform no clock reads.

The native handler guard covers its whole execute method, including catch blocks.
A CoreUpdate guard wraps only the application interface's set_camera call after
successful argument retrieval. Handler/Core guard nesting tracks outermost wall
time without double-counting nested guards; multiple handler invocations inside
one dispatch are rejected as ambiguous when deriving residuals. Independently
nested dispatch scopes have independent results. One paired result is attached to
the existing gesture stamp after dispatch returns, preserving late-gesture IDs.

CameraTimings records dispatch wall time and, for a single valid handler match,
handler wall time and dispatch-minus-handler. A single Core call also provides
Core wall time and handler-minus-Core. Differences are taken before aggregation.
Missing/ambiguous/inconsistent pairs increment unpairedDispatches and omit derived
samples. Outside-handler time also includes caller string conversion, Framework
checks and surrounding callbacks, so it must not be labeled pure parser time.

Mesh edge display
-----------------

The shaded surface may be tessellated for nonlinear elements. Its tessellation
edges are hidden. A separate line actor extracts the original element edges
from the published mesh, retaining quadratic midside nodes. Opaque surfaces
occlude internal edges. This changes only the display; result connectivity and
field arrays remain untouched. Both actors follow the representation visibility.

Camera preview dispatch
-----------------------

Dragging, wheel previews and Esc restoration call the application camera setter
directly on the GUI owner thread. They retain validation, camera/scene revision
guards and presentation-paced coalescing. With camera journaling enabled, release
restores the baseline directly and submits the final absolute camera through the
normal command dispatcher once. Source/result actions retain their command path.
Timing reports show direct camera updates separately from journal command dispatch.

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
