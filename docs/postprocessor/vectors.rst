Vector plots
============

Vector plots draw arrow glyphs over an existing source or filter representation
in a 3D view. They are off by default. The mesh keeps its independent coloring,
visibility and opacity settings. Different views retain independent vector
settings for the same source.

Use the Vectors controls beside the deformation controls in result navigation.
Select a vector field, then enable Vectors. Options controls arrow length,
scale, sample distribution, density and coloring. Changes use the normal Cubit
command route and errors appear in Cubit's messages.

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
The existing legend visibility control also controls the vector legend.

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
