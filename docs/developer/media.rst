Framework media utilities
==========================

``Framework/Media`` builds the standalone ``cubitsim_media`` shared library
before either workspace. Its dependencies are Qt Core/Gui and Threads; there are
no workspace, VTK, Cubit or Framework-host dependencies. This avoids linking
workspace modules back to the main Framework host, which already owns them.

All classes have paired header/source files:

* ``AnimationExportDefinition``: output format, dimensions, FPS, frame count,
  destination, GIF looping, flattening color and caller metadata.
* ``AnimationExportJob``: owner-thread image-production callback; asynchronous
  capability checking, PNG writing, encoding, progress, cancellation and cleanup.
* ``AnimationOutput``: same-filesystem staging and no-overwrite publication.
* ``PngSequenceWriter``: numbered PNG files and a timing/provenance manifest.
* ``GifAnimation``: moved from CalculiX/Utility; the same ordered-frame,
  cancellation/progress API remains available to topology export.
* ``Mp4Animation``: FFmpeg/libx264 capability checking and encoding through
  ``QProcess`` argument arrays, with no shell command construction.

``Utility/gif.h`` is included only by ``Framework/Media/GifAnimation.cpp``.
Its existing public-domain attribution is retained. GIF delays use differences
between rounded cumulative centisecond timestamps, avoiding the old constant-delay
speed error at rates such as 30 or 60 FPS. The helper explicitly flattens alpha and
accepts a loop flag; CalculiX callers retain default looping behavior. Its old
private writer guard is replaced with a local RAII deleter, avoiding another
class with inline implementation.

MP4 explicitly selects libx264, never proprietary/platform/hardware encoders.
``CUBITSIM_FFMPEG`` overrides PATH discovery. Worker-side checks reject nonfree
builds and require the exact libx264 encoder before the first image is produced.
FFmpeg is optional and is not redistributed by this patch. Licensing references
and operational requirements are in :doc:`../framework/media`.

The generic job retains bounded image memory and processes one PNG write at a
time. Captured files live in a private directory next to the final destination.
Encoding runs after capture completes; cancellation cleans staging without
publishing. A publication flag resolves cancellation arriving after publication
as successful completion. Destination paths are made absolute when the job starts,
so later working-directory changes cannot redirect output. Errors and completion
callbacks return to the owner thread; the media library never logs through Cubit.

Postprocessor owns ``AnimationCaptureDefinition`` and ``AnimationCapturePlan``
in Core, a command handler and application-interface service, and the export dialog
and image producer in GUI/Capture. The plan uses immutable source snapshots,
separate result/output FPS and absolute camera samples from the frozen view.
Export-only rotation overrides do not modify live settings. ``render_image``
shares image-capture rendering while PNG encoding for movie frames is moved to
the worker. Reset/source refresh/close/project load cancel through the interface;
module shutdown clears callbacks and joins the job before unloading.

Verification
--------------

``Framework/Media/Tests`` is a standalone Qt-only CMake test project. Postprocessor's
selection tests also include ``AnimationCaptureTests``. These tests are added but
were not built or run. Required native checks include all formats, Unicode paths,
GIF delay/loop behavior, unsupported encoders, cancellation during each phase,
late cancellation/publication, output races, cleanup and plugin shutdown. Check
CalculiX topology GIF behavior after relocation and Postprocessor's independent
camera FPS, multiple passes, alpha, annotations, deformation and empty filtered
frames. No encoder executable, build or OpenGL render was run for this change.
