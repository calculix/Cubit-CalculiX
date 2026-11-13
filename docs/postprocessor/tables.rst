Tables and DAT output
=====================

DAT output can be spatial (node, element or integration-point values) or
nonspatial (totals, section records, summaries and other tables). The source
format is provenance, not a reason to duplicate geometry or interpolate values.

The source tree lists Blocks / Element sets, Node sets, Side sets / Surfaces,
Integration points, Results and Tables directly below each source. Published
model IDs and known element formulations are shown; INP groups with no Cubit ID
do not receive an invented ID. Unclassified data remains under Other datasets.

Matching DAT node/element output reuses a captured model/INP group only when its
association, case-folded name and exact member IDs agree uniquely. Differently
scoped subsets remain separate, labelled DAT output. Field provenance and masks
are retained on the shared group. Integration-point groups remain distinct.

Commands and view behavior
--------------------------

.. code-block:: text

   post table open "Forces / LOAD_TARGET" source "first_run"
   post table field "<exact logical field key>" source "first_run"
   post open file "C:/results/forces.vtt" name "forces"
   post view type 3d

Source defaults to the active source. Native table names and exact catalogue keys
are accepted. Ambiguous names report matching exact keys. Field names are also
accepted when unique; tree actions use exact logical keys.

Double-click a native table to open it. Spatial results offer Open as table in
their context menu. The table opens in a resizable pane to the right of the
existing viewport, inside the middle graphics column. The viewport stays visible
and navigable; no additional renderer is created. The table title's close button
and View -> 3D view both submit ``post view type 3d`` and close the pane. Activating
a spatial field from the tree or Field toolbar also closes it, including when
that field is already selected. Coloring preferences and table closure publish
atomically. Palette/range edits do not close the table. File ->
Import result files accepts VTT and uses the standalone file command. Independent VTT files are tables
only; their column names do not establish links to Cubit nodes or elements.

Native tables preserve columns and row ordering. Vector-valued columns display
all components within their cell. Spatial tables adapt existing arrays, exposing
dataset ID, original node/element ID, IP number when applicable, and stored field
components. Masks and missing/nonfinite values display Unavailable. Values are
not averaged or extrapolated. Integer identifiers retain 64-bit precision.

Frame tables and spatial-field tables show all matching frames together, with
Frame, Step, Increment and Time columns. Frame navigation highlights all rows of
the active frame without replacing history rows or changing the user's selection.
Missing frames contribute no fabricated records and display an active-frame status.
Global and step-summary tables retain their own scope and occur once, with no
invented time or frame. Standalone VTT columns remain as supplied. Frame selection
also works for table-only sources without a mesh representation.
Mesh animation is disabled while a table is displayed. Table sources and models
retain immutable snapshots; closing, refreshing or resetting a source clears the
active table reference and releases old view data on refresh.

The Qt model reads visible cells on demand. It does not materialize a second full
vtkTable for spatial fields. The model retains exact numeric sorting roles, but interactive header sorting
remains disabled pending approval of journalable sort commands. The current
Qt view supports at most INT_MAX rows per opened table/frame; larger tables are
rejected rather than silently truncated. Charts, column filtering and bulk export
are future features with commands to be agreed separately.

Verification
------------

The added verification cases cover native tables, global/frame scope, table-only
navigation, exact integer identities, validity masks, Qt row access and standalone
VTT round-trip. They have not been compiled or executed in this patch. Native
Cubit command parsing, Qt layout and large-table responsiveness still require
verification after an authorized build.


Column sizing
--------------

Columns initially fit their headers and formatted values, sampling at most 1,000
rows distributed across the table. Longer values in newly visible rows widen
columns after scrolling; a full large-table scan is not performed per frame.
Manual column resizing is preserved for the same logical table/schema. Complete
cell values and headers are available as tooltips. Numeric precision is unchanged.
Unchanged panel refreshes do not reset the table model or scroll position. Closing
the pane releases its retained table data. Splitter widths are GUI layout state;
the existing persisted table reference determines whether the pane reopens.


All-frame row indexing
-----------------------

Tables appear directly below the Tables tree item. Scope remains in tooltips,
including global/summary scope; Frame tables is no longer an intermediate group.
A logical frame-table family is concatenated in frame order, preserving each
frame's source row order. The adapter holds row segments pointing at immutable
native tables or spatial arrays. It never creates a combined VTK table or copies
all result rows. Cell access and row-to-frame lookup use the segment index.

Compatible native tables with changing columns use a union of column names and
occurrence positions. Missing columns display Unavailable. Metadata column name
collisions are labelled Result Frame/Step/Increment/Time. Exact integer IDs,
validity masks and numeric precision are retained. Combined row counts above the
Qt INT_MAX limit are rejected before opening; sums are checked for overflow.
Active-frame updates repaint row backgrounds/fonts without a model reset, keeping
selection, manual widths and scroll position. This patch does not add plotting,
render capture or result-annotation commands.
