Model setup captures
====================

CalculiX can render a PNG of the current undeformed finite-element mesh and its
load/BC setup independently of the visible workspace. Factory defaults are
**1920 x 1080 pixels (Full HD)**, white background, black labels and a fitted
isometric camera. PNG captures are separate from the topology animation feature.

Commands
--------

::

   ccx capture model file "model.png"
   ccx capture setup file "setup.png"
   ccx capture loads step 1 file "step_1_loads.png"
   ccx capture bcs step 1 file "step_1_bcs.png"
   ccx capture setup step 1 view front labels on file "setup.png" overwrite
   ccx capture setup view top labels off width 1920 height 1080 file "top.png"

``model`` captures only the mesh. ``setup`` adds both loads and BCs; ``loads``
and ``bcs`` add the corresponding group. ``step`` identifies a model step, not a
result increment. Omit it for entire-model definitions. Views are ``iso``,
``front``, ``back``, ``left``, ``right``, ``top`` and ``bottom``.

Existing files require ``overwrite``. File output uses a temporary file and
commit, preserving the previous destination on write failure. Parent directories
must exist. Width and height must be integers in 64..8192, with at most 32 million
pixels. Other settings use capture defaults, not the current viewport settings.

Capture does not activate CalculiX or change the user's camera, displayed
groups, selection, picking mode or drawing settings. It requires the live Cubit
GUI application thread and an available VTK OpenGL backend. No screen-grab
fallback is used when rendering is unavailable.

Scope and representation
------------------------

Only the **current meshed model** is supported. Captures do not claim to depict
the historical model of a completed job. Current-model Report integration is
described in :doc:`../report/setup-images`; job reports must establish model/job
provenance before using these images.
Result pictures and tables remain Postprocessor responsibilities.

Step captures reuse the existing resolved step conditions, including inherited
and replaced conditions. Amplitudes are references rather than evaluated time
histories. Unsupported custom condition overrides return an error. Step-specific
mesh activation/removal is not simulated: the base image is the current complete
undeformed mesh. Capturing a pure model overview validates the requested step ID
but does not add its conditions.

Mesh faces use existing corner connectivity; curved/high-order faces are
approximated by corner polygons. Unmeshed CAD and unsupported elements are not
rendered as substitutes. The image contains no result deformation or contours.
Symbol size is 10% of the largest mesh extent in world coordinates. Cameras use
parallel projection with Z up, except top/bottom views. Front looks from -Y;
right looks from +X; top looks from +Z. Labels overlay the image after readback;
text occlusion, collision avoidance and automatic text wrapping are not provided.

Code and verification
---------------------

``CalculiXCaptureInterface`` accepts a ``CalculiXCaptureRequest`` and returns
PNG bytes. Failure leaves the previous output unchanged. ``CoreDrawCapture``
builds local geometry, sharing ``CoreDrawEntityGeometry`` with the interactive
load/BC drawing path and ``CoreDrawScope`` with step drawings. The framework
``DrawingInterface::capture`` delegates to ``DrawingCaptureRenderer`` with a
private render window. It does not submit or activate the captured scene.

The command is responsible for paths, PNG extension checks and overwrite policy.
The renderer batches primitives by kind and color and encodes pixels using Qt.
It has no dependency on CalculiX feature types, steps, jobs or Report documents.

Optional regression cases are included in the existing
``CUBITSIM_DRAWING_LABEL_TESTS`` target. No compilation or execution was performed
while builds were prohibited. When builds are permitted, verify:

* Default PNG dimensions are 1920 x 1080 and labels are black.
* Every camera and content mode; labels on/off; mixed force/moment and BC symbols.
* Model/step parity with interactive drawings, including inherited conditions.
* Capturing while Report or Postprocessor is active preserves their view and picks.
* Existing-file refusal, explicit overwrite, unwritable destination and failed writes.
* Missing mesh/step, custom overrides, unsupported elements and unavailable graphics.
* Repeated captures release resources; ordinary load/BC drawing still works afterward.
