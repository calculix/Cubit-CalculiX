Shared animation output
========================

Framework owns reusable GIF, MP4 and PNG-sequence output utilities. They do not
depend on a solver, workspace, VTK or Cubit. CalculiX topology uses the shared GIF
helper; Postprocessor supplies frozen rendered images to the same media layer.

All encoders used by this feature are open-source implementations:

* GIF: the bundled public-domain gif-h implementation in ``src/Utility/gif.h``.
* PNG: Qt's PNG image writer, preserving alpha for PNG sequences.
* MP4: an external FFmpeg executable using the software libx264 H.264 encoder.
  Proprietary OS/hardware encoders are not selected. FFmpeg builds configured
  with ``--enable-nonfree`` are rejected.

For MP4, install a redistributable open-source FFmpeg build with libx264 and put
it on PATH, or set ``CUBITSIM_FFMPEG`` to the executable's absolute path. The app
does not download or install encoder executables. Capability checks run on a
worker before capture starts. Missing FFmpeg does not prevent GIF/PNG export.
libx264 is GPL software and FFmpeg builds incorporating it use GPL terms; preserve
the applicable license/source notices when distributing those binaries. See
`FFmpeg licensing <https://ffmpeg.org/legal.html>`_ and
`VideoLAN x264 <https://www.videolan.org/developers/x264.html>`_. Qt deployments
must retain their applicable Qt and PNG-library notices as well.

Animation jobs keep only one captured image and one pending write at a time.
PNG writing and GIF/MP4 encoding happen on workers; the caller controls where
images are produced. Output is staged next to its destination and existing files
or directories are not replaced. Cancellation requests are asynchronous, and
cleanup finishes before job completion is reported. Library teardown joins its
worker before code can be unloaded.

PNG sequences contain numbered frames plus ``animation.json`` with dimensions,
FPS, duration and caller-provided metadata. GIF timing is quantized to
centiseconds with distributed rounding. Standard MP4 uses opaque H.264/yuv420p
frames and even dimensions. User-facing Postprocessor options are described in
:doc:`../postprocessor/capture`.

The implementation has not been built or executed for this change. See
:doc:`../developer/media` for source ownership and verification.
