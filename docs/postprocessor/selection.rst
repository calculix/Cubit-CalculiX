Selection and probing
=====================

Choose Node, Element or Integration point with the three icon buttons beside Fit.
The active mode stays pressed; tooltips identify each button.
Element is the default. Left click selects one original entity per view; left
drag retains the configured camera action. Clicking empty background clears the
selection. F, Esc and mouse drag bindings retain their camera behavior. Multiple
selection and box selection are deferred.

Only visible result geometry is pickable. The triad, undeformed overlay, edge
actors and selection highlight do not intercept picks. Node/IP picks are checked
against occluding geometry. Nodes are picked from original mesh-point markers,
including higher-order midside nodes. Elements use integral solver IDs copied
through surface extraction; intermediate surface indices are never used as
input-mesh indices. Generated tessellation points are not solver nodes.

Hovering shows an orange element contour or enlarged point marker before a pick.
The magenta highlight marks the committed selection. Hover feedback is local to
the viewport, does not change the selection or probe, and is not journaled.
Hover queries are coalesced at 40 ms intervals and stop during camera gestures.
Leaving the viewport, changing the view/frame/mode or moving the camera clears
preselection. An empty or occluded hit clears the confirmed selection on click.

Commands
--------

.. code-block:: text

   post pick mode node
   post pick mode element
   post pick mode integrationpoint
   post pick node 42 dataset "Mesh"
   post pick element 120 dataset "Mesh" source "first_run"
   post pick integrationpoint 3 element 120 dataset "Integration points"
   post probe
   post pick clear

Entity commands require a dataset ID or quoted published name. Source defaults
to the active source. GUI picks include an explicit source and prefer unique
dataset names; duplicate names use IDs. Journal entries store original identity,
never screen coordinates. Picking another source makes it active. Changing mode
retains the current selection until another pick, an empty-space click or
``post pick clear``.

Quote decimal entity IDs above 9007199254740991 for exact 64-bit transport::

   post pick node "9007199254740993" dataset 0

GUI commands do this automatically. Unquoted numeric IDs are restricted to the
exact integer range of Cubit's double-valued numeric command interface.
Integration-point indices are in [1,2147483647].

Readout and lifetime
-------------------

Properties -> Selection / Probe shows the same information printed by post probe:
source, dataset, original identity, step, increment, time and current field values.
Results -> Probe selection invokes the command. Node/IP selections show original
and deformed display coordinates. Elements have no single intrinsic position.

Point values come from stored point arrays. Element values come from cell arrays;
for nodal fields an element probe reports the valid nodal minimum/maximum without
averaging or extrapolation. Masks, missing fields and nonfinite values are
respected. Unavailable values never become zero. Vector magnitude uses declared
XYZ columns. Palette ranges and limit colors do not alter numerical samples.
Conflicting duplicate identities report ranges or multiple positions rather than
silently selecting a first tuple.

Frame changes update the readout and highlight. Missing entities remain selected
but unavailable until a compatible frame returns. Hiding a dataset hides its
highlight while retaining selection and queryable data. Source refresh, closure
and reset clear selections; reset restores Element mode. Highlights never affect
camera Fit bounds or rebuild source mesh actors.

Selection requires integral OriginalNodeId, OriginalElementId and, for integration
points, IntegrationPointId arrays. The CalculiX converter already publishes them.
Missing or floating-point IDs are rejected; VTK tuple indices are not substitutes.
Ordinary finite-element cell outlines are supported; general VTK polyhedron
outlines are deferred. Original mesh nodes and integration points use rounded,
shaded 6-pixel markers in screen space. Node markers follow deformation and use
the selected nodal field's palette, masks and limits. Cell-associated fields do
not invent values at nodes. Source/dataset visibility also controls its markers.

Build and native graphics/parser validation remain outstanding for this patch.
