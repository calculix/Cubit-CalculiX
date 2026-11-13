Load and boundary-condition labels
==================================

The CalculiX drawing toolbar has a checkable **Labels** button. It changes label
visibility without hiding the load and boundary-condition symbols. The button
uses the same command as the command line::

   ccx display labels off
   ccx display labels on

Label content and placement can be configured with readable commands::

   ccx display labels loads on
   ccx display labels bcs on
   ccx display labels ids on
   ccx display labels names on
   ccx display labels values on
   ccx display labels amplitudes on
   ccx display labels leaders on
   ccx display labels scope selected
   ccx display labels scope all
   ccx display labels offset 0.75
   ccx display labels color black

Factory defaults show IDs and names for loads and BCs, with black text and leader
lines. Defined values and amplitude references are hidden initially. Drawing
options provide defaults for new/reset models; commands change the current model.
The current label settings are saved with the CalculiX drawing state in the
project's HDF5 data. Label geometry and selections are rebuilt rather than saved.

Labels preserve the distinction between ``free``, ``0`` and ``FIXED`` boundary
components. Amplitudes are references, not evaluated time histories. No units are
inferred. Step drawings use the surviving resolved step conditions; values that
differ between targets are marked ``varies by target``.

One label is anchored to an actual target point per drawn entity. Offset is a
world-coordinate distance of ``offset factor * symbol size``, along the positive
coordinate diagonal. It is not a pixel distance or automatic collision avoidance.
Picking the label's anchor or leader selects its owning symbol; text bounds are
not estimated for picking.

Implementation and verification
-------------------------------

``DrawLabelSettings`` owns defaults, validation and the persisted JSON shape.
``DrawLabelData`` carries typed annotation metadata. ``CalculiXDrawLabels`` reads
workspace settings through existing load, BC, mesh and drawing interfaces and
formats the native label primitives. ``CoreDrawScope`` provides resolved step
values and anchors. ``CoreDrawScene`` decorates copies of cached symbol geometry,
so refresh and selection changes cannot accumulate duplicate labels.

Drawing schema version 2 stores label settings. Older development drawing schemas
are not migrated. No Report or Postprocessor screenshot path is added here.

Optional C++ regression coverage is in
``src/CalculiX/Core/drawing/tests`` and is enabled by
``CUBITSIM_DRAWING_LABEL_TESTS``. It covers strict settings decoding, boundary
value text, amplitude references, nonuniform step values, atomic geometry
composition, visibility, selection and duplicate prevention. These tests still
require a build and execution when compilation is permitted. Runtime verification
must additionally check native text rendering, toolbar synchronization, project
save/load, model/step switching and selection in Cubit.
