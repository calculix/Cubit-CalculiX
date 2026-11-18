Vector plots
============

Vector plots draw arrow glyphs over an existing source or filter representation
in a 3D view. They are off by default. The mesh keeps its independent coloring,
visibility and opacity settings. Different views retain independent vector
settings for the same source.

The VV2 button immediately after Component toggles vectors and opens Vector settings
in the right dock. The first eligible field is selected by default. The panel contains
arrow length, scale, sample distribution, density and coloring. There are no
inline field dropdown or Options popup. Changes use the normal Cubit command
route and errors appear in Cubit's messages. Vector legend properties opens a
separate page in the same dock.

Supported fields
----------------

The result provider must explicitly declare physical XYZ components. CalculiX
currently supplies this metadata for FRD DISP, DISPI, FORC, FORCI, VELO and FLUX,
and DAT displacements, forces, velocities and heat flux. Derived columns such
as ALL are excluded from direction and magnitude. Scalars, tensors, principal
stresses and arbitrary three-column arrays are not automatically vectors.
Local-coordinate DAT tables are not eligible spatial fields; a known basis
transformation would be needed before drawing them in global directions.

Commands
--------

Select a field before enabling the plot::

  post vector field "DISP" origin "FRD" source "first_run"
  post vector on source "first_run"
  post vector length proportional source "first_run"
  post vector scale 1.5 source "first_run"
  post vector samples surface source "first_run"
  post vector density maximum 2000 source "first_run"
  post vector density every 1 source "first_run"
  post vector color magnitude source "first_run"
  post vector off source "first_run"

Source defaults to the active source. Commands configure its active
representation in the active view, including a selected filter representation.
Field selection accepts the existing origin, association, set and dataset
qualifiers. Ambiguous names require qualifiers. Selecting a field retains the
enabled state; turning the plot off retains all settings.

``length uniform`` gives nonzero vectors equal lengths. ``samples all`` includes
interior samples. ``density maximum 0`` disables the count limit. ``density every
N`` retains approximately one sample in N using a deterministic identity hash;
it does not depend on transient VTK array ordering. The count limit still applies.
``color solid`` uses a neutral blue instead of the vector magnitude palette.

Scale and units
---------------

The reference arrow length is 5 percent of the first spatial source frame's
visible-dataset bounding-box diagonal. It is independent of deformation,
filtering, the current animation frame and camera position. The reference
magnitude is the maximum of the selected vector across all source frames and
visible datasets, before filtering. Proportional length is::

  reference_length * scale * magnitude / reference_magnitude

Uniform length is ``reference_length * scale``. Scale must be finite, greater
than zero and at most 1000. A zero-size source has no automatic arrow length.
These are visual lengths; force and heat-flux values are not displacements.
The magnitude legend uses the Postprocessor's presentation units and the same
all-frame reference. Capture uses its frozen presentation-unit context.
The vector legend has independent visibility, palette, range, limits and position.
It uses the same compact bottom-right layout and normal fonts as the result
legend, in a neighboring slot. Drag either legend independently. Movement previews
are silent; release journals one final position. Escape restores the previous
position. Double-click opens that legend's own properties page.

Vector legend properties applies selections immediately and numeric values on
editing completion. Defaults restores Core legend defaults while retaining vector
field and arrow settings. Low color and High color define a custom HSV gradient
that is sampled using the selected band count. Reset palette colors restores the
selected built-in palette.
Changing presentation units also converts vector legend bounds and limits.

Examples::

   post properties vectors
   post properties vectorlegend source "first_run"
   post vector legend position 0.6 0.08 source "first_run"
   post vector legend auto source "first_run"
   post vector legend palette rainbow source "first_run"
   post vector legend palette low color 0.1 0.2 1 source "first_run"
   post vector legend palette reset source "first_run"
   post vector legend range allframes source "first_run"
   post vector legend defaults source "first_run"

Legend colors match arrow magnitude colors. Hiding the result legend does not
hide the vector legend. Both legends and custom colors persist in projects and
are retained by image, report and animation captures.

Sampling and animation
----------------------

Nodal arrows start at displayed node positions, element arrows at parametric
cell centers and integration-point arrows at their published positions.
Surface sampling retains original exterior face nodes, including higher-order
face nodes, or the elements owning those faces. Integration-point samples use
their explicit positions regardless of the surface setting. No element-to-node
averaging is performed. Nonconforming interfaces can appear as exterior faces.

Deformation moves the anchors; the vector components retain their reported
global directions. Clip, Section and Threshold supply their displayed geometry.
Nodal vectors on generated clip/section points use the existing interpolation
pipeline. Such points do not acquire fabricated solver node identities.

Zero, masked and nonfinite vectors are skipped. A missing field hides the
arrows with a display warning while retaining the selection. The sample rank
uses dataset and original entity/integration-point identities when available.
Generated filter samples use displayed coordinates as a fallback, so their
sampling can change with geometry. Changes in validity or retained geometry
can also change the selected subset.

Animation updates vectors with each result frame. Arrow instances are bounded
by the configured maximum; zero explicitly requests all. Camera fitting includes
arrow extents. Images, reports and GIF/MP4/PNG animation captures use the same
scene rendering. Mesh entity picking remains available underneath the arrows;
individual arrow picking is not implemented.

Persistence
-----------

Project files retain vector settings per representation. Dataset-specific
selections use durable dataset bindings during restoration and refresh. A
missing or ambiguous dataset during refresh rejects the replacement source
rather than reusing its numeric index. Older projects without vector settings
load with vectors off. Runtime arrows are rebuilt, never serialized.
