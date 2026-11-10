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

Paired handler timing checks after build authorization
-----------------------------------------------------

* Rebuild after authorization, enable the existing camera diagnostics option and
  repeat continuous rotation. Confirm camera handler, Core camera update, dispatch
  outside handler and handler outside Core appear with matching normal-call counts.
  Keep the previous whole-dispatch metric for comparison. Expect zero unpaired
  dispatches for successful GUI camera commands in the normal native path.
* Verify total >= handler >= Core per invocation and residuals before aggregation;
  never subtract independent maxima. A large outside-handler maximum implicates
  surrounding dispatch work; a large Core or handler maximum directs further
  inspection inside the application. Wall time remains distinct from GPU time.
* Exercise invalid arguments, command rejection and nested enabled/disabled probe
  scopes. Missing handlers must not fabricate zero handler samples. Failed Core
  updates still count their executed Core scope. Unwinding must restore context.
* Run the updated camera timing and controller tests after compilation is permitted.
  They cover paired arithmetic, missing/multiple matches, scope bounds, nested
  isolation and recovery. They have not been compiled or run by the agent.

Original mesh edges and direct navigation
-----------------------------------------

After the next authorized build, verify a HEX20 example displays element
boundaries without face diagonals. Actual tetrahedral meshes must retain their
triangular boundaries. Check hide/show/close for both surface and edge actors.
DisplayTests adds a bowed midside-node HEX20 case and checks 24 line segments
(12 quadratic edges), unchanged source topology and partition actor lifecycle.
CameraTests checks direct previews make no text dispatch calls, coalesce an input
burst into one Core revision, retain Esc cancellation, and submit one optional
normal journal command on release. These additions have not been built or run.

With camera diagnostics enabled, compare frame interval maxima during a long
drag. Direct camera updates have their own timing metric; command-dispatch
samples should occur only at a journaled commit, not on every preview frame.

Reset regression checks
-----------------------

DisplayTests covers source/selection/runtime cleanup, actor release without
rendering, single-view restoration, advancing revisions, rejection of stale fit
completion, repeated reset and opening results afterward. CameraTests covers
reset during an active gesture, Esc afterward and preservation of user options.
After an authorized build, also test native Cubit reset with Postprocessor
visible, hidden and without its GUI started. Confirm only the original reset
command is journaled and no stale geometry returns after switching workspaces.

Frame navigation patch
----------------------

Added DisplayTests cases for active/explicit source frame selection, unavailable
frames and invalid ordinals, idempotence, hidden visibility, preserved camera,
scene geometry replacement and independent source instances. NavigationTests
checks tree refresh without command callbacks, disabled missing frames, expansion
preservation, external frame selection, explicit source callback arguments,
sequence numbering across gaps and reset cleanup using Qt offscreen widgets.
These additions have not been compiled or executed.

After an authorized native build, open a multi-increment job. Try both forms of
``post frame``, tree selection and all navigation buttons. Confirm journal entries
contain absolute frame numbers and explicit source names for GUI navigation.
Verify invalid/empty frames leave the display unchanged, hidden sources stay
hidden, an established camera stays fixed and reset removes stale controls.

Result coloring and dock regression checks
-----------------------------------------

ColoringTests covers threshold equality, disabled/re-enabled retained limits,
crossed thresholds, invalid RGB, missing values, bands/reversal, extreme signed
normalization, vector magnitude excluding derived columns, mask handling and
immutable inputs. DisplayTests adds scalar mapping, maximum-limit color, palette
updates without scalar-geometry replacement, missing-field frame changes and
legend/actor cleanup on reset. Added cases have not been compiled or executed.

After an authorized build, test both nodal and element fields, known displacement
magnitude and integration-point datasets. Verify palette/range commands, limit
on/off, masks, negative values, constant fields, missing fields and frame changes.
Compare thresholds against numeric result values in the model's own units.
Test double-click and toolbar command journaling, independent source settings,
hide/show/reset cleanup and legend readability on light/dark backgrounds.
Float/close/redock Result navigation, switch workspaces and restore it through
View. Confirm Command Line remains available and the dock placement is retained.

Grouped results, slider and options defaults
--------------------------------------------

Updated NavigationTests for one result item across repeated frames/datasets,
stable selection/expansion, unavailable result state, gap-preserving slider
positions, coalesced previews, one release commit and reset cancellation.
ColoringTests covers compatible group identity and incompatible associations.
CameraTests adds staged mouse-page default reset without command submission,
discarding unsaved defaults and navigation-journal option publication.
Added/updated cases have not been compiled or run.

Native checks after an authorized build: migrate an old saved layout and verify
Result navigation is directly above Command Line; float/move it and verify that
placement survives workspace changes/restart. Scrub a multi-frame, multi-dataset
source with navigation journaling on/off. Confirm no frame tree or repeated DISP/
STRESS entries, one recorded release command when enabled, no preview entries,
correct group coloring, safe reset/source-switch cancellation and unchanged camera.
On each plugin options page, stage defaults, close without Save, then repeat and
Save; check only the page's changed settings are applied through commands.

Animation and prefixed options
------------------------------

DisplayTests adds Core animation defaults, restarting from the final frame,
invalid FPS without mutation, live loop/FPS updates, source-change invalidation
and reset cleanup. NavigationTests drives Qt timer timeout signals directly to
cover skipped frames, silent previews, one final commit, loop wrap, live FPS and
loop changes, end-of-sequence stopping and reset. These tests have not been
compiled or executed.

After an authorized build, verify all four options pages sort together under
Cubit-Sim, open/save correctly and are removed correctly on plugin shutdown.
Play a result with gaps in its frames; check 10 FPS default, pause, restart at
last, live FPS changes and enabling/disabling Loop. Switch sources/workspaces,
close/hide the navigation dock and reset during playback. Compare journal output
with navigation recording on and off: no timer frames, one final absolute frame
when enabled. Confirm the camera and result-color settings remain unchanged.

Display Properties regression cases
------------------------------------

DisplayPropertiesTests adds Qt offscreen cases for refresh/edit without command
submission, scientific fixed ranges, paired limit edits with valid intermediate
commands, band count and legend settings, drafts surviving frame changes, external
coloring refresh, disabled-limit value retention, invalid drafts without mutation,
partial command failure, accepted-but-rejected submissions, stale source changes
and reset disabling the page. The mocked dispatcher applies every emitted command
through the application interface and deliberately triggers synchronous refresh.
These additions have not been built or executed.

After an authorized build, check Properties at narrow widths, source metadata,
field/component readout, all palettes, reversal, bands, range modes and scientific
notation. Apply minimum/maximum values and colors together, then toggle each
limit independently. Verify command journaling and that animation preserves
pending edits. Test external commands, source/view changes, reset, color-dialog
context changes and an unavailable field/frame. Confirm failures show the actual
committed settings and leave no misleading success state.

Compact navigation and all-frame ranges
----------------------------------------

DisplayTests adds grouped multi-frame/multi-dataset bounds, masked outlier and NaN
exclusion, explicit dataset scope, unchanged mapper range during frame navigation,
cache invalidation after refresh and all-invalid data without fabricated bounds.
ColoringTests checks streaming range accumulation against masked vector extraction.
DisplayPropertiesTests covers the All frames editor choice and command transport.
These additions remain uncompiled and unexecuted.

After an authorized build, check that Result navigation occupies a single toolbar
row beneath the main Postprocessor toolbar and leaves Command Line at normal
width. Check slider/play/FPS/Loop, moving/floating, View visibility and workspace
activation. Apply Current frame, All frames and Fixed ranges to results whose
extrema differ between frames. Verify All frames keeps legend endpoints fixed
while the result colors change. Refresh results and confirm the global bounds
update; test masks, unavailable fields and independent limit highlighting.

Deformation toolbar and geometry
-------------------------------

DeformationTests covers declared XYZ columns with an extra magnitude column,
masked/NaN/overflow fallback, immutable original coordinates and arrays, dataset
scope, mixed nodal/integration-point automatic displacement, ambiguous/missing
roles and manual overrides. Application/scene checks cover transformed and overlay
fit bounds, independent coloring and current-frame fallback.

Toolbar cases check absence of Apply, immediate checkbox and field commands,
refresh without submission, scientific scale debounce, silent previews, exactly
one final normal command, duplicate completion suppression, incomplete exponent
focus-out, frame changes during scale input, external updates, rejected commands,
accepted-without-mutation detection, hide cancellation and reset. Timer signals
are driven directly rather than waiting in tests. These cases have not been built
or executed.

After an authorized build, confirm Deform, vector, Scale and Undeformed share the
navigation row without Apply. Test immediate toggles, field selection, typing
pauses, Enter, focus loss and scientific notation. Verify previews are absent from
the journal and the final scale appears once. Test source/workspace changes, reset,
missing displacement, overlays and Fit. Camera movement must retain the existing
direct preview path without rebuilding geometry.

Dataset visibility and names
----------------------------

DisplayTests adds ID/name visibility, duplicate-name rejection, literal all name,
unknown selectors without mutation, metadata-order independence, single-grid data
without metadata, actor/scalar-geometry retention, current/all-frame/fixed ranges,
Fit, absent-frame isolation, parent master toggles, hiding all, refresh reset of
masks and retained result ownership. NavigationTests checks dataset checkbox
callbacks using a sparse ID and signal-free preference refresh. These cases have
not been compiled or executed.

After an authorized build, exercise show/hide/show-only by both ID and name, plus
Show all and the parent master checkbox. Check names with spaces, numeric names,
quoted all, duplicate names and unavailable frames. Verify native parser behavior
and journal replay, including the GUI's unique-name preference and numeric fallback.
Confirm context menus survive/cancel safely around source refresh and workspace
changes, legends and Fit follow visibility, and other sources/views retain their
own settings. Source refresh must reset visibility overrides. No build or native
parser validation was performed for this patch.

Selection/probe and toolbar split
--------------------------------

SelectionTests adds exact 64-bit IDs and command formatting, point/cell/IP values,
masked samples, deformed positions, element nodal ranges without averaging,
missing-frame reappearance, hidden status, toolbar/readout refresh, CPU provenance
picking/occlusion, actor retention, click-vs-fast-drag handling, and source
closure/refresh/reset. The offscreen target is registered for Windows/Linux with
the existing VTK/Qt test dependencies. These cases have not been built or executed.

Static checks cover source registrations, API signatures, delimiter/brace balance,
UTF-8 and whitespace. No compiler, linker, CMake configure, test executable, GL
Render or native Cubit parser was run.

After an authorized build, check the shared row order Field selection ->
Deformation -> Navigation, independent View toggles, floating/moving and workspace
switching. Confirm 6-pixel rounded point datasets retain colors/deformation. Pick
nodes, original elements and IPs on undeformed/deformed data; test background Clear,
F/Esc and all drag bindings. Verify no picks during drags and no camera commands
from plain clicks. Check hidden geometry, overlays, high-DPI displays, curved
higher-order faces, absent IDs, quoted 64-bit IDs and journal replay. Probe values
must match original arrays and masks, update with frames, and never report a
rendering triangle index as an element ID. Selections/highlights must clear with
refresh, closure and reset without changing Fit bounds.

Original-node picking and preselection follow-up
----------------------------------------------

The selection checks now cover copied 64-bit original element IDs on a quadratic
cell surface, original mesh-node picking, background/occluded misses, and hover
without Core selection changes. They have not been compiled or run in this patch.
Native verification should include midside nodes, deformed nodal colors, dataset
visibility, hover clearing on camera/frame/mode changes, and empty-space clearing
of both the committed highlight and the probe readout.

Group tree, DAT aliases, tables and hover values
----------------------------------------------

New source-level checks cover DAT fields bound to a captured model nodeset,
published formulation/ID metadata, read-only node adjacency probes, spatial table
masks and exact IDs, native/global/frame tables, table-only navigation, and VTT
read/write fixtures. No build, compilation, native parser or test execution was
performed for this extension. Verify tree checkbox/selection actions in Cubit,
including the formerly duplicated load_target entry, before claiming the reported
hang is resolved. Also verify standalone VTT paths on Windows and Linux, source
closure/reset, missing-frame tables and return to the retained 3D camera.


Project persistence and category visibility
-------------------------------------------

The project persistence implementation and outstanding native verification are
specified in :doc:`postprocessor-project-persistence`. New standalone cases in
``ProjectPersistenceTests.cpp`` cover Core/codec restoration and tree category
behavior. Reader persistence cases compare durable result fingerprints across
HDF5 round-trip. VTT cases reject changed file content on restoration. These new
cases were not built or executed for the persistence patch, per the current
no-compilation instruction. Native save/open and staged HDF5 recovery still need
Windows/Linux integration verification.


Source tools and independent imports
------------------------------------

``ImportTests.cpp`` adds cases for exact VTU node IDs and numeric arrays,
comparison-source isolation, failed-import session preservation, VTPC dependency
changes, and catalogue selection versus explicit Add. Native toolbar binding,
FRD/DAT/INP conversion and menu interaction still require Cubit integration checks.
These cases were added without building, compiling or executing them, per the
current instruction. Verify Windows and Linux paths (including non-ASCII names),
DAT-only tables, INP includes, incompatible mesh rejection, source refresh,
missing dependencies on reopen, and journal replay of the new commands.


Integration-point reference outlines
-------------------------------------

``ReferenceMeshTests.cpp`` covers exact 64-bit parent IDs, repeated IP parents,
higher-order topology/coordinates, identical overlapping blocks, ambiguous and
missing parents, and immutable inputs. These cases were added without compiling
or executing them. Native GUI verification must additionally hide the parent
block, display deformed IPs with Undeformed enabled, toggle the IP dataset and
Undeformed, and confirm that reference edges do not enter the pick list.


External DAT spatial fallback
------------------------------

Native regression: import an FRD mesh omitting a reference node and a DAT whose
nodal output includes that node, without INP. Confirm that the FRD is displayed,
unlinked nodal blocks and IP blocks lacking exact formulations appear as tables,
and IDs/values/frame navigation are preserved. Then import with a compatible INP
containing the reference node and formulation, and confirm spatial association.
Also cover mixed linked/unlinked DAT blocks, DAT-only tables, malformed records,
and incompatible INP rejection. Fallback diagnostics are deduplicated across
frames. These native checks have not been executed; no build was performed.
DisplayTests now asserts the Node and AllFrames Core defaults; it was not run.


New-source display defaults
----------------------------

``SourceDefaultsTests.cpp`` covers blocks-only initial visibility, skipping hidden
fields during automatic result selection, vector Magnitude/scalar component
selection, first-category fallback, initial Results expansion/field selection,
manual collapse retention and retained-source visibility preservation. The
category persistence fixture explicitly enables node sets before testing bulk
hide operations. Cases were added but not compiled or executed; native GUI and
journal replay verification remain outstanding.
