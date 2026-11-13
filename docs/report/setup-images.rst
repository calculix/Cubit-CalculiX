CalculiX model setup images
===========================

Current-model reports can embed CalculiX setup captures automatically. The
default is an undeformed model overview plus one combined load/BC setup picture
for every selected analysis step. For models without steps, the setup picture
shows the entire model's definitions.

Images use **1920 x 1080 pixels (Full HD)**, a white background, black labels
and a fitted isometric camera by default. They are generated through
``CalculiXCaptureInterface`` without activating the CalculiX workspace or changing
the user's viewport. Result plots and result tables remain Postprocessor-owned.

Configuration
-------------

The report content tree provides the following switches:

* Model overview image (on).
* Combined setup image per step (on).
* Separate load image per step (off).
* Separate BC image per step (off).
* Labels in setup images (on).

The **Images** tab in Report settings provides the camera and pixel dimensions.
Choices and numeric ranges come from the workspace template. All options are
saved by the existing framework HDF5 report-profile store. Readable commands are
also supported::

   report configure "ccx.images.model" "true"
   report configure "ccx.images.setup" "true"
   report configure "ccx.images.loads" "false"
   report configure "ccx.images.bcs" "false"
   report configure "ccx.images.labels" "true"
   report configure "ccx.images.camera" "front"
   report configure "ccx.images.width" "1920"
   report configure "ccx.images.height" "1080"
   report configure "steps" "1,3"
   report profile save "Standard"
   report update

Camera values are ``iso``, ``front``, ``back``, ``left``, ``right``, ``top`` and
``bottom``. Dimensions must be integers in 64..8192; the combined limit is 32
million pixels. The image provider checks that combined limit before collection.
The report supports at most 256 setup captures and 128 MiB of their embedded
base64 image data.

``steps`` selects one-based ordinals in model/export order. It is not a list of
step IDs. For IDs 2, 7 and 42, selection ``1,3`` includes IDs 2 and 42 in both
the model tables and images. Captions include the ordinal, actual ID and name.

The settings dialog submits a single readable settings command. Template-specific
fields use repeated quoted pairs; Report Core validates and commits the complete
patch atomically::

   report configure settings option "ccx.images.camera" value "front" option "ccx.images.width" value "1920"

Source provenance and persistence
---------------------------------

Supported generated jobs freeze setup data during export and bind it to the final
input manifest and successful solver launch. Their reports use archived input for
tables and frozen scenes for pictures, even after the current model changes or
original files are removed. See :doc:`../developer/calculix-job-snapshots`.

Imported jobs can retain input without a typed setup snapshot. Unsupported or
unverified setups produce diagnostics; current geometry is not substituted.
Turning off all setup image types removes that diagnostic. Topology and adjoint
contexts do not yet retain heavy input/setup archives.

A later launch requires explicit report generation; ordinary update does not
silently switch runs. Profile camera and label changes can be applied to frozen
scenes. Setup snapshots do not establish Postprocessor result-file lineage.

Capture failures such as unavailable graphics or unresolved step conditions add
diagnostics for the affected images. Invalid settings, invalid selected step
ordinals and resource-budget failures stop collection and preserve the previously
committed report. Regeneration replaces the provider's image root and diagnostics
without accumulating duplicates. Stable section IDs keep report notes associated
with their section.

Generated PNGs are embedded in the report document and saved in the project.
Opening a saved report, previewing it or exporting its PDF/ODT uses those stored
images and does not request a new capture. ``report update`` regenerates them.
The existing preview layout and PDF/ODT section-image exporters are reused.

Implementation and verification
-------------------------------

``CalculiXReportImageSettings`` owns template image options, validation and ordinal
mapping. ``CalculiXReportImageProvider`` implements collection and single-image
rendering through the existing framework provider registry. Generic Report option
metadata describes choices and integer ranges; Report GUI/Core do not interpret
CalculiX step or feature types. The CalculiX template revision is 2; no development
profile migration is added.

Regression cases cover image-provider collection failure, replacement without
duplicates, option/request JSON, generic choice/range validation, readable GUI
commands, profile HDF5 persistence, report project persistence and section-image
PDF/ODT exports. CalculiX cases cover gapped step IDs, capture request decoding,
pixel budgets and unverified job diagnostics. These cases were added without
building or executing them while compilation remained prohibited.

After building is permitted, additionally verify model reports with several steps
and nonconsecutive IDs, profile save/load, step-specific image content, missing
mesh/graphics diagnostics and generation while another workspace is active.
