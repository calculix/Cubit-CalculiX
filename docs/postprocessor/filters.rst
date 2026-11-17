Clipping, sections and thresholds
=================================

A named clip retains one side of a plane. A named section intersects the mesh
with a plane. A named threshold retains elements or discrete integration points
matching scalar limits. All filters are derived outputs: they do not change the
solver mesh or published result arrays. Numeric controls are available in separate
Clip, Section and Threshold pages in the right properties dock. Interactive plane
dragging and filter chains are deferred.

Commands
--------

Names are unique across filters. Quote names containing spaces::

   post clip create "Cutaway" source "snap_fit" origin 0 0 0 normal 1 0 0 keep positive
   post section create "Section A" source "snap_fit" origin 25 0 0 normal 1 0 0
   post clip plane "Cutaway" origin 10 0 0 normal 1 0 0
   post clip keep "Cutaway" negative
   post section plane "Section A" origin 30 0 0 normal 0 1 0
   post filter hide "Cutaway" view "view1"
   post filter show "Cutaway" view "view1"
   post filter rename "Section A" "Mid-section"
   post filter remove "Mid-section"
   post filter list

Creation uses the active view and optionally the active source. It copies the
source representation's frame, coloring, deformation and dataset visibility,
then hides that original representation in the active view. Other views are
unchanged. Filters have separate per-view representations; show/hide affects the
specified view (the active view when omitted). Plane edits affect the named
filter in every view. Removing a filter removes all its representations and does
not automatically show its original source. Use the normal source show command
when the original is required again.

Selecting an existing filter in its control tab shows and activates it. Existing
frame, coloring, deformation and dataset controls then address that active output.
All committed changes use commands, including numeric plane edits and visibility.
The small plane outline is an editing aid and is excluded from PNG capture.

Mathematical and data contract
------------------------------

The normal is finite, nonzero and normalized before publication. With displayed
position p, origin o and unit normal n, signed distance is d = n dot (p - o).
Positive clipping retains d >= 0; negative clipping retains d <= 0. A section is
the intersection d = 0. The plane uses displayed/world coordinates **after**
deformation. It stays fixed while frames or deformation scale change.

An empty intersection is a valid empty output. Invalid parameters fail before
replacing the committed definition. VTK clipping may subdivide cells internally;
displayed mesh edges come from original element edges or section boundaries,
so subdivision diagonals are not presented as solver mesh edges.

Original node markers retain exact solver IDs. Synthesized section/clip vertices
have no solver node ID and cannot be picked as original nodes. Retained cells
carry original element provenance. Element probes and history plots refer to
original source results, not a newly invented element. Section extrema labels
identify interpolated display samples. Invalid nodal samples become NaN before
interpolation; generated display values cannot make invalid input valid.

Integration points remain discrete points during clipping. Section cuts do not
invent integration-point surfaces or extrapolated integration-point values.

Current-frame ranges use the filtered output. All-frames ranges keep the source
field's range for stable comparison and are identified as Source all frames in
the legend. Source buffers and other representations remain unchanged.

Ownership, caching and persistence
----------------------------------

Core filters own definitions; representations own frame and display settings.
Evaluation caches one current geometry output per retained representation,
keyed by source snapshot, definition revision, frame and deformation. Changing
frames replaces that cache entry; no generated arrays are written to the project.

Project save/load stores filter definitions and per-view representation settings.
Unavailable sources retain those definitions and their saved cameras until the
source can be resolved. Closing a source removes its dependent filters; reset
clears the filter registry and cached outputs. Viewport display, fit and PNG
capture use the same evaluator.

Implementation
--------------

::

   Postprocessor/
   +-- Core/filters/
   |   +-- FilterDefinition.hpp         plane, kind, side, Core defaults
   |   +-- PlaneOperations.cpp          normalize_plane / valid_filter
   |   +-- ClipFilter.cpp               evaluate_clip
   |   +-- SectionFilter.cpp            evaluate_section
   |   +-- FilterRuntime.cpp            evaluate_filter / filter_parent_edges
   |   +-- PostprocessorCoreFilters.cpp create_filter / change_filter / evaluate_output
   |   +-- FilterCodec.cpp              filters_to_json / filters_from_json
   +-- Interfaces/PostprocessorFiltersInterface.cpp
   +-- Commands/filters/                clip / section / filter commands
   +-- GUI/Filters/
       +-- Clip/PostprocessorClipControls.cpp
       +-- Section/PostprocessorSectionControls.cpp
       +-- PostprocessorPlaneOverlay.cpp

VTK FiltersGeneral is included in both Windows and Linux dependency declarations.
The Windows Cubit VTK adapter feature marker includes plane-filters so that an
older prepared adapter is regenerated on the next authorized build. Presence of
host VTK libraries alone does not validate binary compatibility. This patch has
not been compiled or exercised in Cubit.

Native coordinate-unit basis
----------------------------

Origin controls show native source units, while normals are dimensionless.
Plane definitions retain the known native length basis. Refreshing into a
different coordinate scale invalidates the derived output; recreate that plane
in the new source basis. Changing workspace presentation units does not alter
plane coordinates. See :doc:`units` for legacy/unknown-unit handling.

Threshold filter
----------------

The Threshold toolbar button (selected mesh cells) opens its properties in the
right dock. It creates a named feature output of a source, like Clip and Section.
Select a scalar component or a declared vector magnitude, optionally qualify one
dataset, and enable a lower limit, upper limit, or both. Both enabled limits must
be finite and lower must not exceed upper. Bounds are inclusive; equal bounds
select exact matches. Threshold selection is independent of result coloring.

For nodal results, **All nodes** is the default: every node of an element must
match. **Any node** retains the whole element when at least one node matches.
Both modes preserve the whole original element, including higher-order nodes;
there is no scalar interpolation or geometric clipping. Invalid masks, NaN and
infinite samples never match. Element results use each element's own value.
Integration points remain individual vertex cells and ignore the nodal rule.

For example, with limits 30 through 80, an element with node values 20, 40, 70,
90 is removed by All nodes and retained by Any node. An element with values 40,
60, 70, 50 is retained by both rules.

Commands (each example is one command line)::

   post properties threshold
   post threshold create "Stress band" source "snap_fit" field "S" component "MISES" lower 100 upper 250 nodes all
   post threshold configure "Stress band" field "S" component "SXX" lower 100 upper 250 nodes any
   post threshold field "Stress band" field "S" component "SXX"
   post threshold limits "Stress band" lower off upper 250
   post threshold nodes "Stress band" all
   post threshold reference "Stress band" on
   post filter select "Stress band"
   post filter disable "Stress band"
   post filter enable "Stress band"
   post filter hide "Stress band" view "view1"
   post filter rename "Stress band" "High stress"
   post filter remove "High stress"

Field selectors accept the same origin, association, set and dataset qualifiers
as coloring. Use ``component magnitude`` only for declared physical vectors.
``configure`` replaces field, limits and node rule atomically. ``field`` changes
the field while retaining the numeric limits; numbers are interpreted in the
new field's current presentation unit. ``limits`` replaces both bounds in the
current presentation unit. ``off`` disables that bound; both cannot be off.

Every committed GUI action submits a command through the workspace's normal
Cubit command-line dispatcher. Programmatic refresh submits no commands. Core
validates settings and command handlers report failures with Cubit messages;
Threshold does not open validation message boxes. Rejected changes preserve
the last valid definition and output. New-filter controls prepare the arguments
until Create threshold submits the complete creation command.

Limits retain quantity, unit, dimension and temperature-role metadata. Workspace
unit changes convert their numbers while preserving physical selection. Each
frame is evaluated using its native field units and the retained bounds. Unknown
units remain explicitly unknown; incompatible physical units reject evaluation.
Dataset-qualified definitions retain kind, name and model identity for refresh
and project reload, rather than trusting a reusable numeric dataset ID.

A dataset/frame without the selected field contributes empty geometry and a
warning. An empty match is valid. Nonselected datasets contribute empty geometry
while the filter is enabled. Bypass shows the original geometry. Optional Show
removed portion displays the complementary cells in gray. Removed cells cannot
be picked as retained output. Original node/element/IP IDs remain exact; source
buffers are immutable. Node markers, edges, entity highlights, undeformed overlay,
current-frame ranges and capture use the same retained geometry. All-frame ranges
continue to describe the original source, as for plane filters.

Threshold settings, node rule, unit descriptors, dataset identities, enabled state
and per-view representations are saved with the project. There is no generated
geometry in the saved definition. Existing projects containing only plane filters
remain readable. Closing a source removes its threshold features; reset clears
all filters. Inputs remain sources; filter chaining is deferred.

Threshold implementation files (relative to ``src/Postprocessor``)::

   Core/filters/ThresholdDefinition.hpp / .cpp
   Core/filters/ThresholdFilter.hpp / .cpp
   Core/filters/ThresholdCodec.hpp / .cpp
   Core/filters/FilterDefinition.hpp / .cpp
   Commands/filters/PostThresholdCommand.hpp / .cpp
   Commands/filters/ThresholdCommandText.hpp / .cpp
   GUI/Filters/Threshold/PostprocessorThresholdControls.hpp / .cpp
   Interfaces/PostprocessorFiltersInterface.cpp

This implementation has not been built or exercised in Cubit. Verification cases
are included in ``ThresholdTests.cpp`` and ``FilterTests.cpp`` for the existing
``postprocessor_filters`` test target; compilation remains pending authorization.
