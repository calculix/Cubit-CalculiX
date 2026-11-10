Tables and DAT output
=====================

DAT output can be spatial (node, element or integration-point values) or
nonspatial (totals, section records, summaries and other tables). The source
format is provenance, not a reason to duplicate geometry or interpolate values.

The source tree separates Mesh, Integration points, Results and Tables. Mesh
contains Blocks / Element sets, Node sets and Side sets / Surfaces. Published
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
their context menu. The table replaces the active 3D view; View -> 3D view returns
to the retained scene and camera. No additional viewport is created. File ->
Import result files accepts VTT and uses the standalone file command. Independent VTT files are tables
only; their column names do not establish links to Cubit nodes or elements.

Native tables preserve columns and row ordering. Vector-valued columns display
all components within their cell. Spatial tables adapt existing arrays, exposing
dataset ID, original node/element ID, IP number when applicable, and stored field
components. Masks and missing/nonfinite values display Unavailable. Values are
not averaged or extrapolated. Integer identifiers retain 64-bit precision.

Frame tables follow frame navigation; absent tables show an unavailable state
instead of stale values. Global and step-summary tables retain their own scope.
Frame selection also works for table-only sources without a mesh representation.
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
