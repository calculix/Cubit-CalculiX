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
figure preservation and codec compatibility. They have not been run. Native GUI
acceptance must verify viewport/plot rendering, high-DPI and large output, alpha,
labels/triad/legend/marker placement, unavailable graphics backends and Report
PDF/ODT image export on both Windows and Linux. No build was run for this patch.
SVG and PDF chart export remain deferred; PNG figures can be embedded in Report
PDF/ODT exports.
