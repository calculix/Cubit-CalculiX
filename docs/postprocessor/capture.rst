PNG captures and Report figures
===============================

The Screenshot button saves the active viewport at its physical pixel dimensions.
Render image opens a dialog for a viewport or named plot, output dimensions,
aspect-ratio lock, background and overlays. Add to Report uses the same choices
and adds an embedded, immutable PNG to the active generated Report. Dialog edits
are local drafts; accepting submits one journalable command. Cancel changes nothing.

Commands
--------

.. code-block:: text

   post capture view file "C:/results/beam.png"
   post capture view "view1" file "C:/results/beam-large.png" width 1920 height 1080
   post capture plot "Displacement history" file "C:/results/history.png" width 1600 height 900
   report image add postprocessor view "view1" caption "Stress distribution"
   report image add postprocessor plot "Displacement history" caption "Displacement over time"

Dimensions default to 1920 by 1080. Width and height must be supplied together;
each side is 64..8192 pixels, with at most 32 million pixels overall. PNG payloads
are limited to 32 MiB. Exports require a new filename in an existing directory;
existing files and symlinks are refused. A completed temporary file is published
only after encoding and writing succeed.

Both targets accept ``background current|white|transparent`` and ``legend on|off``.
Viewport targets also accept ``labels on|off`` and ``triad on|off``; plot targets
accept ``marker on|off``. Omitted overlay settings preserve the current state.
A plot's current background is its plot background. Transparency preserves labels,
legend and triad foreground colors chosen for readability against a light backdrop.

Snapshot contract
-----------------

Core copies session definitions and retains shared immutable result handles.
A private application and private render window evaluate that frozen state on
the host GUI thread. Capture never activates a workspace, changes the live camera,
advances a frame, changes selection, or resizes a live widget. Animation is stopped
only in the copy. Transient selection/hover highlights are excluded. Result labels,
extrema labels and the plot frame marker use the same painting code as the live
view. Output scaling uses logical view dimensions as its reference; line widths,
points and text scale with the smaller output/reference dimension ratio. Camera
orientation and projection are retained; a changed output aspect ratio changes
the visible horizontal extent. A view with no saved camera fits its private mesh.

PNG text metadata ``CubitSim.Capture`` stores provenance: source identities,
frame/step/increment/time, field/component, coloring, limits, deformation, camera
or plot definition/resolved axes, output size and overlay settings. No arrays are
serialized into the manifest. Metadata is bounded to 4 MiB.

Report persistence
------------------

A generated Report must already be selected. Capture appends to its dedicated
Figures section without switching workspaces or generating content. Figures retain
their encoded PNG and provenance across regeneration, update and project save/load;
they do not follow later result, camera or source changes. The first version allows
64 user figures per report and enforces the existing 64 MiB report-session save
limit before insertion. Capture/provider/validation failures preserve the committed
Report and output files. Existing projects without image provenance or typed visual
references remain readable.

Verification status
-------------------

Regression cases cover request limits, incompatible overlays, detached state,
shared result ownership, no-overwrite publication, Report insertion failures,
figure preservation and codec compatibility. They have not been run by the agent. The user has verified viewport capture
with deformation, legend, triad and labels; plot capture with legend and active-frame
marker; and Report insertion followed by project save/reopen and regeneration
with unchanged figures. This confirms that exercised build only. Remaining GUI
acceptance must verify viewport/plot rendering, high-DPI and large output, alpha,
labels/triad/legend/marker placement, unavailable graphics backends and Report
PDF/ODT image export on both Windows and Linux. No build was run for this patch.
SVG and PDF chart export remain deferred; PNG figures can be embedded in Report
PDF/ODT exports.

Presentation unit provenance
----------------------------

Snapshot evaluation uses a captured presentation-unit context rather than live
workspace preferences. Manifest version 2 includes this context and each saved
representation's bounds-unit metadata. See :doc:`units`. Saved PNG assets remain
unchanged when workspace units are later edited.

Viewport captures include the enabled distance ruler using the snapshot's scene
unit and presentation context. View IDs, scene units and ruler visibility are
recorded in capture provenance. Perspective scale is explicitly local to the
camera focal plane.


Render animation
------------------

The filmstrip Render Animation button follows Render Image and precedes Add to
Report. It opens a separate animation dialog with view/source, GIF/MP4/PNG-sequence
format, dimensions, aspect lock, result-frame range, result FPS, output FPS,
passes, background, legend, labels and triad. Rotation defaults to the selected
representation's current settings; clear Use current axis rotation settings to
supply export-only X/Y/Z flags and degrees per pass. Export changes no live frame,
camera, coloring, deformation or animation settings.

Result FPS controls the duration of each mesh frame. Output FPS samples camera
motion independently, so a 10-FPS result sequence can produce a 60-FPS MP4/PNG
sequence with smooth rotation. Missing mesh frames are skipped; selected source
frames are one-based positions, not solver increment numbers. Other visible
sources in the view stay frozen. The view must have a fitted camera. Rotation
continues across encoded passes, using the stored world-axis order X, Y, Z.

The PNG destination is a new directory, containing frame_000000.png onward and
animation.json (FPS, dimensions, frame count and capture provenance). PNG preserves
transparency. GIF/MP4 require Current or White backgrounds; alpha is flattened.
GIF supports 1..100 FPS with centisecond timing distributed to avoid cumulative
rounding drift. MP4 supports 1..120 FPS, H.264/yuv420p and even dimensions. All
formats allow 64..8192 pixels per side, at most 32 million pixels and 100000
output frames. Existing output files/directories are never overwritten.

GIF's Loop playback option repeats the finished file. To close a half-turn camera
path seamlessly, encode two passes (180 degrees + 180 degrees). A file player
always restarts a finite file at its first image; encoded passes themselves do
not reset the camera phase. Standard MP4 has no automatic loop attribute.

MP4 requires a separately supplied open-source FFmpeg build with libx264. Set
``CUBITSIM_FFMPEG`` to the executable, add it to ``PATH``, or place it manually
in ``tools/ffmpeg`` beside the loaded media library. The exporter checks the
override first, the manual drop-in second, and ``PATH`` last. Capability checking
runs before frame rendering.
The exporter rejects --enable-nonfree builds and never selects proprietary OS or
hardware encoders. No executable is downloaded automatically. GIF uses the
bundled public-domain gif-h encoder; PNG uses Qt's PNG implementation.

Export is asynchronous. A progress window reports rendering/encoding; Cancel
export submits a normal command. Reset, source close/refresh, project reload and
module shutdown request cancellation. Capturing stays on the GUI/OpenGL thread;
PNG writing and GIF/MP4 encoding run on workers with bounded image memory. Output
is staged beside its destination and published only when complete. Cancellation
before publication leaves no final output. If cancellation arrives after successful
publication, completion is reported as successful. Failures use Cubit messages.

Commands::

   post capture animation view "view1" format gif file "result.gif" fps 30
   post capture animation view "view1" format mp4 file "result.mp4" source "first_run" width 1920 height 1080 first 1 last 20 resultfps 10 fps 60 passes 2
   post capture animation view "view1" format png file "result_frames" fps 60 background transparent
   post capture animation status
   post capture animation cancel

The file argument denotes a directory for PNG sequences. Optional loop on/off,
legend on/off, labels on/off and triad on/off use the image capture conventions.
For export-only axis settings, provide all axes together::

   post capture animation view "view1" format gif file "turn.gif" rotatex off anglex 180 rotatey off angley 180 rotatez on anglez 360

The bundled Windows encoder was checked for libx264. No build or native Cubit
capture verification was run; Linux execution still requires a Linux machine.
