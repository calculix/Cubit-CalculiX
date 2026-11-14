Clipping and section cuts
=========================

A named clip retains one side of a plane. A named section intersects the mesh
with a plane. Both are derived outputs: they do not change the solver mesh or
published result arrays. This first implementation provides numeric controls in
the Filters dock, with separate Clip and Section tabs. Interactive plane dragging
and filter chains are deferred.

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
