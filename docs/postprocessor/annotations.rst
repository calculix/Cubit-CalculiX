Result labels and extrema
=========================

Labels belong to a Postprocessor view. They bind an original entity ID and a
fixed result field/component, and update their values and leader anchors as the
frame or deformation changes. Changing the active color field does not retarget
existing labels. Names identify definitions in commands; the displayed text
contains entity IDs and values, not the internal label name.

Creating and managing labels
---------------------------

Select an entity and use the Add result label toolbar button, or right-click
an entity and choose Create result label. GUI creation submits an explicit
generated name, source, dataset, field/component and view. These operations use
the same command handler as typed commands::

  post label add node 9
  post label add "tip" node 9 dataset "Block_1" source "first_run" field "DISP" component magnitude
  post label add "stress" element 12
  post label add "ip stress" integrationpoint 3 element 12
  post label rename "label_1" "tip"
  post label hide "tip"
  post label show "tip"
  post label remove "tip"
  post labels hide
  post labels show

All commands accept an optional trailing ``view "id"``. Otherwise they target
the active view. Omitted sources and fields use the current source/color field.
An omitted dataset must resolve uniquely among datasets containing that entity
and a valid requested value. Ambiguity is an error, never a first-match choice.
Names are case-sensitive and unique per view. An omitted name uses the smallest
unused positive ``label_N``. User names cannot begin with ``@``, contain control
characters, exceed 256 UTF-8 bytes or contain both quote delimiter types.
Quote decimal entity IDs above 9007199254740991 for exact 64-bit transport.

Dragging and command recording
------------------------------

Drag label text with the left mouse button; the leader remains attached to the
displayed entity. Positions are normalized coordinates of the text's bottom-left
corner relative to the drawable viewport, measured from bottom-left, in [0,1]::

  post label position "tip" 0.75 0.30

Text bounds are clamped inside the viewport when rendered. Mouse previews are
coalesced to the next render and use ``silent_cmd``. Release records one final
position command, even if its position matches the last preview. Esc restores
the starting position silently. Competing annotation changes, reset, source
removal or geometry/frame changes invalidate gesture ownership. Annotation
dragging consumes its mouse events before camera navigation.

Extrema
-------

The Min/Max toolbar button and viewport context menu submit::

  post labels extrema on
  post labels extrema off

Markers use finite, valid original samples of the active field/component in the
current frame, limited to visible datasets of the active source. They do not use
the palette's All frames range or stress-limit colors to determine extrema.
Dataset-ID order followed by original tuple order breaks equal-value ties.
Invalid masks, nonfinite values and tessellation vertices are excluded. No
extrapolation or averaging of result values is introduced.

The two extrema labels can also be dragged::

  post label position "@minimum" 0.05 0.15
  post label position "@maximum" 0.65 0.85

``post labels hide`` hides every label while retaining individual settings;
``post labels extrema off`` disables only extrema. Turning extrema back on
retains their text positions.

Anchor and persistence contracts
-------------------------------

Node and integration-point leaders use their original result positions with
the current deformation applied. An element leader uses the arithmetic mean
of its original cell-node positions after deformation. This is a geometric
placement convention, not an interpolated result or physical centroid. An
element's nodal results retain the existing probe's valid min/max range.
Conflicting duplicated positions have no unique leader anchor.

Definitions, visibility, fixed fields, exact entity IDs, durable dataset identities
and text positions are saved with the project. Missing sources, datasets or
samples retain definitions as unavailable; ordinals are not silently reused for
unrelated datasets. Source refresh resolves durable dataset identities again.
Explicit source removal deletes its labels; reset clears all annotation state.
Older projects without annotation data receive Core defaults. Calculated extrema,
rendering objects, caches and in-progress gestures are not persisted.

Value evaluation is cached independently from camera projection and label text
positions. Rotating the camera or dragging text does not rescan result arrays.
Plotting and image capture are separate features and are not added by this patch.
