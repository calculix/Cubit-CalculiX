Shared drawing service
======================

The cubitsim_drawing library renders workspace-owned primitives and symbols.
It knows no CalculiX feature enums, step models or solver DOF numbering.
CalculiXDrawingAdapter owns these conversions.

FrameworkContext owns DrawingService and passes DrawingInterface to modules.
WorkspaceManager controls the active renderer owner. A single picker restores
native Cubit pick modes and invalidates deferred callbacks on frame replacement,
owner switches, callback replacement, reset and teardown.

A DrawingFrame has unique workspace/kind/id/context keys, named layers, explicit
style, visibility and optional highlighting. Empty selection context matches all
contexts of the same workspace/kind/id. Context is otherwise opaque. Coordinates
are finite world coordinates. Solver-specific scaling and defaults remain in
workspace Core. Native drawing-set names encode workspace and layer identifiers.

Exclusive layers temporarily hide native geometry/mesh/simulation. Previous
visibility is restored on switch or detach. Native display-refresh observation
uses callbacks so each workspace owns policy. Commands, toolbars, tree mappings,
persisted scopes and drawing settings remain workspace responsibilities.
Topology solving and animation control remain in CalculiX.

Validation after builds are allowed
----------------------------------

Check primitive/symbol parity, BC/load/equation display, model/step/custom scopes,
highlighting, hover, native pick-mode restoration, native refresh, topology native
visibility restoration, identical local IDs in two workspaces, switching with
queued picks, reset, GUI reattachment and shutdown. Headless initialization must
not access a viewport. Validate against supported Cubit/VTK on Windows and Linux.
