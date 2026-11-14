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
   post table field "STRESS" origin "FRD" association node source "first_run"
   post open file "C:/results/forces.vtt" name "forces"
   post view type 3d

Source defaults to the active source. GUI commands use readable table/field names.
Native tables accept optional ``scope frame|global``, ``type "metadata type"`` and
``entry N`` qualifiers. ``type`` matches TableType, falling back to ResultKind;
``type ""`` explicitly matches absent type metadata. ``entry`` is a positive,
1-based occurrence within a name/type/scope family in provider order, not a frame
number or a global catalogue ordinal. Reordering otherwise identical tables in the
provider output changes that occurrence's meaning, as it did for existing keys.
Use an entry qualifier only when metadata cannot distinguish the tables.

Field tables reuse ``origin "FRD"`` and ``association node|element|integrationpoint``.
They show the complete field, so selector uniqueness never depends on a currently
selected component. Native and spatial selectors are resolved across all frames;
changing the active frame does not select a different table family. Ambiguity
reports readable candidates and leaves the view unchanged. Existing exact keys
remain accepted for replay and are still stored in project definitions.

Tree/context actions and returning to the Table tab generate the same readable
commands. Qualifiers are added only when necessary and checked against the exact
stored identity before dispatch; generation never falls back to an opaque key.

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

Table-column plots
------------------

.. code-block:: text

   post plot add table "forces / LOAD_TARGET" x "Time" y "fx" source "first_run" plot "History 1"
   post plot add field "STRESS" origin "FRD" association node x "Time" y "SXX" where "Node ID" equals "1171" source "first_run" plot "History 1"

``add table`` accepts the native table qualifiers above; ``add field`` accepts the
spatial field qualifiers. Column names are exactly those displayed in the table.
Spatial tables use ``Node ID``, ``Element ID``, ``IP`` and ``Dataset``; native DAT/VTT
tables retain their supplied column names. Filters compare exact cell text without
converting large IDs through floating point. A time/frame history must isolate one
entity/dataset identity and at most one matching record per frame. Compound IP or
cross-block identities that cannot be isolated by the single table filter require
an entity-history plot instead. Values are never averaged or silently connected
across different entities.

Table plotting validates the readable target and quoted arguments before creating
a new plot. A source refreshed/replaced while the dialog is open must be reopened.
Saved table curves retain internal keys and a boolean spatial-field flag; older
projects without that flag default to native-table curves. The flag participates
in plot-cache identity. No combined history arrays are serialized or duplicated.
