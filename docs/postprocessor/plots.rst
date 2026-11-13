Result-history plots
====================

Plots open to the right of the independent Postprocessor viewport. Table and
Plot tabs share this area when both are open. The source tree remains a data
catalogue; plot selection belongs to the plot pane and history toolbar menu.

Commands
--------

Create a named plot and add original-entity histories::

  post plot create "Displacement history"
  post plot add node 9 field "DISP" component magnitude
  post plot add node 15 field "DISP" component magnitude name "Second node"
  post plot add integrationpoint 3 element 12 field "stresses" component "SXX"
  post plot add element 12 field "STRESS" component "SXX" reduction maximum

Entity additions accept optional qualifiers in this order: ``dataset ID|"name"``,
``source "name"``, ``field "key"``, ``component magnitude|"name"``,
``reduction minimum|maximum``, ``name "curve"`` and ``plot "name"``. Omission
uses the active source/field/plot; an omitted dataset must resolve uniquely.
Quote decimal IDs above 9007199254740991 for exact 64-bit transport. Element
histories over nodal fields require explicit minimum/maximum reduction. Stored
cell fields use their original element value. No averaging or extrapolation is
introduced. Integration-point identity includes both element ID and IP number.

Right-click a table cell and choose Plot columns to select numeric X/Y columns,
an optional exact-value filter and an existing/new plot::

  post plot add table "forces / LOAD_TARGET" x "Time" y "fy"
  post plot add table "forces / LOAD_TARGET" x "Time" y "fy" where "NodeId" equals "9"

Table additions accept trailing ``source "name"``, ``name "curve"`` and
``plot "name"``. Names or exact table keys are accepted; ambiguity requires an
exact key. Columns must be uniquely named. Filters compare exact source cell
text without converting identifiers through floating point. An unambiguous
Valid column controls row validity. Time/Frame plots with mixed known entity
identities or multiple records per frame require a narrower filter. Use entity
histories for compound element/IP identities that a single filter cannot isolate.

Management and axes::

  post plot show "Displacement history"
  post plot close
  post plot rename "Displacement history" "Displacement comparison"
  post plot curve hide "Second node"
  post plot curve show "Second node"
  post plot curve remove "Second node"
  post plot axes auto
  post plot axes range x 0 1 y 0 10
  post plot export csv "C:/results/displacement.csv"
  post plot remove "Displacement comparison"

Curve, axes and export commands accept ``plot "name"``. Closing hides the pane;
removal deletes its definition. Names are case-sensitive, unique in their scope
(plots in the session, curves within a plot), limited to 256 UTF-8 bytes, and
cannot contain control characters or both quote delimiter types. An unnamed
curve receives the smallest unused positive curve_N.

GUI actions use these commands. Entity history actions supply explicit source,
dataset, field/component, curve name and plot. With no active plot, the GUI first
records a create command. Table/Plot tab changes also submit existing table-open
or plot-show commands; closing or hiding a pane does not bypass Core state.

Interaction and numerical contracts
-----------------------------------

Left drag pans, the wheel zooms around the current range center, and F fits axes.
Only linear axes are supported. Bounds must be finite, increasing and have finite
spans. Previews use silent commands coalesced into the next render; release or
wheel inactivity records one final range command. Esc/focus loss restores the
starting definition, including automatic axes. Native VTK mouse handling cannot
independently mutate persistent axes or control the mesh camera.

Entity curves preserve original frame order, including time restarting between
steps. Missing entities, masks and nonfinite values produce gaps. VTK 9.4.2's
vtkPlotLine bad-point handling splits runs at invalid samples. Native tables
retain source row order; global tables receive no invented frame membership.
Compared sources/columns must use compatible units: no unit conversion is inferred.

The vertical marker follows a compatible source's current mesh/table frame. It
is a presentation overlay and cannot change automatic bounds. Arbitrary numeric
X columns or mixed axis meanings do not receive an invented time marker.

Only requested scalar samples are materialized. Complete result arrays remain
shared immutable source data. The pane caches values by curve input and source
snapshot; axis changes and frame-marker updates do not rescan result arrays.
Core limits are 64 plots, 64 curves per plot and 1,000,000 samples per curve.
Oversized requests fail without truncation.

Persistence and export
----------------------

Projects save definitions, references, curve colors/visibility, axes and pane selection.
Curve buffers, VTK objects and in-progress gestures are excluded. Missing sources
retain unavailable curves/settings. Refresh resolves durable dataset identities;
explicit source removal prunes dependent curves but retains empty plot definitions.
Reset clears plots. Older projects without plot data receive empty defaults.

CSV records plot/curve/source, exact entity/IP IDs, frame/step/increment/time,
X/Y, validity and status. Formatting is locale-independent with 17-digit precision.
Invalid values remain gaps. Existing files or symlinks are refused; a temporary
file in the target directory is completed before publication. Plots without any
sample rows cannot be exported. PNG capture and embedded Report insertion are
described in :doc:`capture`. Direct SVG/PDF plot export remains deferred.
