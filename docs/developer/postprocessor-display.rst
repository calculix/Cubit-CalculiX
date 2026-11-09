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

Per-view frame selection
------------------------

``PostprocessorCore::set_frame(source_id, view_id, increment_index, error)`` uses
a zero-based index into the immutable published result. The application interface
converts the 1-based command ordinal and resolves names and active view. No VTK
arrays are copied or modified. A valid change updates only the representation
index and scene revision; camera and visibility are preserved. A missing
representation is created hidden. Provider reentry, invalid identities and absent
mesh are rejected. Scene synchronization already keys actor groups by retained
result handle plus increment index, so it replaces the relevant geometry.

The tree stores typed item kinds, owning source IDs/names, and catalogue indices
in Qt data roles. Visible labels never supply command source names. Expansion
and selection are restored under QSignalBlocker; command failure restores GUI
controls from committed state. Navigation is a Postprocessor-specific widget;
there is no new shared panel abstraction or solver GUI dependency.

Coloring pipeline and contracts
-------------------------------

ResultColoring belongs to Representation. The application resolves human-readable
field/component names to a dataset ID, exact array name, association and component.
Settings are independently stored per source/view; changing a frame retains them.
Core validates finite ordered ranges and thresholds, normalized RGB, band limits
and declared vector semantics. Source-to-source unit conversion is not performed.

The converter declares vector_components for known displacement arrays. Magnitude
is hypot of those three columns, excluding derived extra columns. Unknown vectors
and tensors require a named component. Element arrays use cell data; nodal and
integration-point arrays use point data on their respective dataset geometries.
Integration-point values are not extrapolated onto the structural mesh.

A display-owned shallow copy receives derived scalar values. Invalid masks and
nonfinite required components produce NaN before surface extraction, conservatively
propagating missing data through nonlinear interpolation. Source arrays and active
scalar/vector selections remain unchanged. The finite range uses original valid
samples, not tessellation-generated samples. Other datasets remain neutral.

A field/component change rebuilds the derived scalar surface. Palette, range,
bands, limit and legend changes reuse that scalar geometry. Nodal scalars are
interpolated before color mapping; element values retain cell association. The
4096-entry mapping table samples the continuous result-to-color function. Visual
limit crossings therefore have lookup-table/raster resolution; numerical threshold
comparisons use full values. The legend samples the palette range separately and
shows enabled limit thresholds in colored labels. This is visualization, not a
pass/fail stress evaluation tool. Equal-valued fields use a central palette color
and a minimally expanded legend interval.

RenderingAnnotation is explicitly required by Windows SDK preparation and Linux
VTK component discovery. Core remains independent of Qt/VTK presentation objects.
WorkspaceView toolbarRowBreaks starts a results row at Field selection. The
Deformation and Result navigation toolbars follow on that same row, each with an
independent object name, View toggle and movable/floatable Qt toolbar.

Default drag rotation direction
-------------------------------

Inner-region Rotate drags use negative horizontal/vertical pixel deltas for
yaw/pitch, restoring the original orbit direction. Outer-region spin uses the
negative pointer angle delta. These mappings apply to the Rotate action (left
drag by default), including reassigned buttons. Mathematical rotate/roll command
conventions and pan/zoom mappings are unchanged.

Grouped catalogue and frame slider
---------------------------------

SourceRuntime caches the immutable logical field catalogue once per published
source snapshot. Group keys exclude dataset and frame IDs while retaining field
origin, set, array identity, association, component layout and vector semantics.
Queries copy the catalogue and update availability from only the selected frame.
Tree, toolbar and double-click application share this Core catalogue and stable
keys; they no longer create result items per frame or per compatible dataset.

FieldSelection has an optional dataset scope and stable group key. No scope means
all compatible datasets. Scene extraction resolves the field separately for each
mesh part and combines finite ranges across those parts. Explicit dataset-scoped
commands retain their original meaning. Incompatible variants are not merged.

The frame slider uses displayable-frame positions. Its zero-delay single-shot
timer consumes only the newest pending position. The owning workspace guards
source/view/result-handle/frame identity before silent preview dispatch. Own
preview refreshes retain the gesture; reset/external changes invalidate it.
A completed drag issues one absolute normal command, unless journal/navigation
is off. Hiding the toolbar restores an owned baseline through the silent path.

Postprocessor navigation is registered as a toolbar rather than a bottom dock.
Its former dock object is no longer constructed, avoiding the expanding layout
area. Framework places the main toolbar first and inserts the declared row break
before navigation on workspace activation.

Animation ownership and scheduling
----------------------------------

AnimationDefinition stores FPS/loop and a transient playing request per
Representation. PostprocessorCoreAnimation validates the 1..120 FPS interval and
requires two displayable frames before playing; starting at the end selects the
first frame. Only one source representation may play in a view. Source selection
and result replacement invalidate prior playback intent. App commands resolve
names through PostprocessorApplicationInterface without solver GUI dependencies.

PostprocessorFrameNavigation owns the presentation timer. It is a precise,
single-shot Qt timer rescheduled after each completed frame; expensive frames
do not accumulate catch-up requests. Silent frame previews retain the existing
source/view/snapshot/expected-frame guard. Matching own-frame refreshes retain
the timer while applying live FPS/loop changes. Reset or a different source
invalidates the local timer. Hiding the navigation toolbar requests pause.

Play/Pause/FPS/Loop widget actions submit PostAnimationCommand. GUI navigation
recording controls Play/Pause and the final absolute frame; no per-frame timer
updates are journaled. The final commit is queued and guarded against source,
snapshot, frame and playback changes before dispatch. Typed commands retain normal
Cubit journaling. Animation is a visible-GUI presentation, not a Core worker loop.

Options labels are prefixed in Framework UserOptionsManager before validation,
registration and storage in its page list. Detach removes exactly those labels;
workspace labels and settings namespaces are not renamed.

Display property drafts and command application
-----------------------------------------------

PostprocessorDisplayProperties is a feature-specific widget within the existing
Properties dock. It owns a copied committed baseline and local controls; it never
mutates Core state directly. Context includes source ID/name, active view,
representation ID and immutable published-result handle, excluding the current
frame so animation does not discard drafts. An external coloring change reloads
the baseline. Modal color dialogs also check ownership before retaining edits.

Apply uses C-locale finite scientific-number parsing and Core valid_coloring.
The field selection stays unchanged; toolbar/tree own field application. The
command planner simulates every intermediate ResultColoring and emits existing
post color/post legend commands with explicit safely quoted source names and
max_digits10 numeric formatting. No new command syntax or GUI storage is added.
The default inactive band count is defined in ColoringDefaults.

If limit values/colors change, currently enabled endpoints are disabled first.
Changed endpoints are configured individually through the existing enabling
value/color command and disabled again before restoring the requested flags.
This also permits configuring retained disabled thresholds that cross the other
endpoint, without ever publishing an invalid enabled pair.

Every dispatched command is followed by a fresh context and exact expected-state
check. Submission acceptance alone is not handler success. Reentrant refreshes
are suppressed during Apply; a changed context or rejected command aborts the
remaining plan. Recovery reloads the committed partial state, with no invented
rollback. Invalid drafts retain their edits and submit no commands.

All-frame range computation
----------------------------

ColorRangeMode::AllFrames uses PostprocessorCoreColorRange through the application
interface. SourceRuntime caches the optional finite range by complete field
selection identity (dataset scope, array, association, component/magnitude and
logical group key). The cache belongs to the immutable published snapshot and
is discarded on source refresh, close or reset. No current-frame index, palette
or limit setting enters the cache key.

Range accumulation streams original array samples using the same validated
array/component/mask reader as scalar display; it does not allocate a scalar
array per scanned frame. Only matching fields in matching dataset parts contribute.
Missing fields are skipped. Malformed arrays/masks produce an error rather than
a partial global range; no valid samples returns nullopt with no fabricated bounds.

Scene synchronization requests global bounds only for visible AllFrames
representations. The color mapper and legend share those bounds, retaining the
current-frame scalar geometry. Cache changes invalidate mapping tables without
changing source arrays. A failed global-range query disables contour mapping
instead of displaying a fallback numeric range as if it were correct.

Representation deformation
--------------------------

ResultDeformation belongs to each Representation, independently of ResultColoring
and AnimationDefinition. Core defaults are disabled, Automatic selection, scale 1
and no overlay. Explicit field selection is retained independently of the mode.
Commands use PostprocessorApplicationInterface; names are resolved there and Core
receives source/view IDs. Field selection uses the existing logical group key,
optional dataset scope and declared vector-component metadata. Cell fields,
unknown vector layouts and repeated/out-of-range XYZ indices are rejected.

SimResults::FieldRole distinguishes Unspecified and Displacement. The CalculiX
converter marks both nodal DISP and its shape-function-interpolated integration-
point displacement at publication. The existing interpolation and validity masks
are unchanged. Field role participates in logical group identity. Postprocessor
contains no CalculiX field-name rules: resolve_deformation_field requires exactly
one role-marked compatible vector for each dataset in each frame. Missing or
ambiguous choices produce a reference fallback, never first-array selection.
Rendering and camera bounds share this resolver through deform_mesh.

Enabling Automatic requires at least one resolvable mesh dataset; other datasets
may still warn and retain reference coordinates. Changes to scale on an already
enabled representation remain possible when a later frame lacks displacement.
Automatic can always be stored while disabled. Switching selection retains the
enabled state; each toolbar control now submits one independent command. Explicit commands
remain usable for untagged imported vectors. No role inference from VTK's active
vector designation or array names, and no exported-file role roundtrip, is added
by this patch.

The mathematical contract is x' = x + scale*u in compatible coordinate units,
without automatic conversion. deform_mesh builds fresh double coordinates for
vtkPointSet inputs, shallow-sharing immutable topology and result arrays. The
original point set and all published arrays remain unchanged. Invalid masks,
nonfinite vectors and overflowing transformed coordinates retain the complete
original point, never a partially transformed XYZ tuple. Missing fields/arrays
or unsupported mesh types retain reference geometry with a warning.

The scene cache key includes source snapshot, frame and deformation settings.
Changed deformation reconstructs surface and original-element-edge pipelines,
invalidating color geometry while retaining independent color settings. Optional
reference edges use a neutral overlay color and no scalar coloring. Current
transformed point sets are owned by RepresentationActors; no per-frame deformation
cache accumulates. Camera-only interaction does not rebuild geometry. Core fit
bounds use the same transformation helper, including original bounds for overlays.

Warnings are retained per actor group, combined for visible representations, and
reported by the viewport only when the message changes. They do not turn a valid
reference fallback into a repeatedly failing scene synchronization.

PostprocessorDeformationControls shares the navigation toolbar row without Apply.
Checkboxes and field selection use normal commands immediately. Scale text uses
a 250 ms single-shot timer and a separate silent-command callback. The widget
retains the edit origin and expected published settings. The original formatting
and incomplete exponents survive refresh and frame changes while ownership and
expected deformation remain unchanged. Enter or focus loss journals the final
value once, even if a preview has already published it; a net-zero edit restores
silently. A focus-out event filter also completes invalid/intermediate input,
for which QLineEdit does not emit editingFinished. Hiding cancels and restores
an owned preview silently. Context changes stop the timer and discard pending
input; callbacks never submit to a newly selected target.

Commands use explicit safely quoted source names. Every dispatch validates the
source/view/representation/snapshot and committed deformation before submission,
then checks readback. Synchronous refresh is suppressed during dispatch. Rejected
changes reload actual settings. A rejected final scale commit restores the edit's
starting value silently when ownership is still valid. Programmatic control
updates do not submit commands. No solver GUI calls or new persistence formats
are introduced.

Per-dataset visibility
----------------------

Representation owns hidden_datasets keyed by actual dataset ID; an empty set
means all enabled. Effective visibility is the source master flag AND absence
from that set. PostprocessorCoreDatasetVisibility validates IDs against the
immutable SourceRuntime mesh catalogue, stages changes atomically and publishes
one display revision only for a changed definition. Show/ShowOnly/ShowAll enable
the parent; Hide preserves its master flag. Source refresh clears masks in every
view, preventing a reused dataset ID from inheriting an old snapshot's visibility.

The catalogue is the union of actual mesh parts across frames. Published names
are matched by ID, never metadata vector position; datasets without metadata
retain numeric identity and measured counts. The application interface resolves
ID/name selectors before Core mutation, rejecting duplicate names. GUI commands
prefer safely quoted unique names, falling back to IDs. Tree rebuilds block
signals. Modal context menus retain source/snapshot/view identity rather than
item pointers, rejecting stale menus after a refresh or reset.

RepresentationActors tags surfaces, original edges and undeformed overlays with
their dataset IDs. Visibility-only revisions update existing actors without
rebuilding geometry. Warning aggregation excludes hidden datasets. Core Fit uses
the same effective-visibility predicate before computing deformed/overlay bounds.
The color mapper keeps per-surface extrema and scalar geometry, recombining visible
ranges without extracting surfaces again. Fixed bounds and independent thresholds
are untouched. Legends consume slots only for visible scoped geometry.

All-frame extrema are cached per field/component/dataset within SourceRuntime,
then merged for the visible selection. Visibility combinations never become cache
keys, preventing combinatorial cache growth. A malformed visible field reports an
error; hidden datasets do not participate in the all-frame query. Snapshot refresh
invalidates all of these numerical caches through a new runtime object.

Selection, probing and original identities
-----------------------------------------

ViewDefinition owns SelectionState (mode, optional EntityReference, revision).
EntityReference contains source ID, dataset ID, kind and original signed 64-bit
node/element identity, plus IP index where needed. Core validates identity against
the current representation frame before publication. Picking another source makes
it active and stops previous-source animation in that view. Selection-only changes
advance selection revision rather than the mesh display revision. Source refresh,
closure and reset clear selections atomically with the corresponding operation;
reset also advances the selection revision seed to invalidate stale tokens.

ResultProbe uses integral OriginalNodeId, OriginalElementId and IntegrationPointId
arrays. Floating IDs and rendering indices are not identity fallbacks. Queries
read masks and scalar/vector semantics through the same single-sample coloring
reader, without allocating whole scalar arrays. Deformed point coordinates use a
single-point equivalent of the display transformation, respecting the same XYZ
indices, masks, finiteness and overflow rules. Element coordinates are undefined;
nodal fields on elements produce valid node extrema, not averages. Conflicting
repeated IDs produce ranges/multiple locations. Missing-frame selections remain
stored with unavailable readouts and no stale highlight.

Surface extraction copies integral OriginalElementId cell attributes through
nonlinear intermediate meshes. The picker reads those solver IDs from the hit
surface cell, never treating intermediate pass-through indices as input indices.
Node picking uses separate original-point actors with unchanged tuple order and
OriginalNodeId arrays, including higher-order midside nodes. IP picking targets
only integration-point datasets. Pick lists exclude edges, reference overlays,
triad and highlights. A camera-direction depth check treats occluded points as
misses; points in front of the surface remain pickable. QVTK click coordinates
are converted to device pixels. Native GL/high-DPI validation remains required;
no Render was run for this patch.

CameraInteraction reports unmodified left clicks below Qt's drag threshold only
after completing the camera transaction. Crossing the threshold on any move marks
a drag even if the cursor returns before a render, preventing accidental picks.
Camera previews retain the approved direct application path; selection uses normal
journaled commands. Exact GUI pick commands quote IDs above the safe double integer
range and prefer unique dataset names. Three pick-mode icon buttons live on the
main toolbar; empty-space clicks or post pick clear clear the selection.
Field selection, Deformation and Navigation occupy separate toolbars in that order
on the results row. No geometry rebuild is needed for a selection-only revision.

Highlights own only small derived point/cell geometry. They are nonpickable and
excluded from renderer bounds; node/IP markers use their own selection styling,
while element outlines use original FE edges. General polyhedron outlines remain
deferred. Mesh-node actors and IP datasets use Core-owned 6-pixel rounded shaded
point defaults, preserving nodal scalar colors, masks, limits and deformation.
Point-associated colors share the surface lookup table; cell-associated colors
are not extrapolated onto node markers. Source arrays are never modified.

Hover reuses the small entity-outline builder with a separate orange, nonpickable
actor. Coalesced 40 ms queries occur only without mouse buttons or modifiers;
camera gestures clear pending feedback. Camera/geometry/selection revisions clear
stale hover. Hover changes neither Core state nor the journal; click commits use
the existing pick/clear commands. An empty or occluded click clears the committed
selection and probe.
