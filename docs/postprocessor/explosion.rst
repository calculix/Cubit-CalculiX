Exploded views
==============

The two toolbar buttons after the filters toggle component and mesh explosion.
Both also open the explosion properties directly in the right dock, without a
dropdown or an extra panel-opening journal command. Explosion is a
presentation transform; geometry, result values and solver IDs remain unchanged
in the published source.

Commands
--------

.. code-block:: text

   post explode
   post explode off
   post explode components on
   post explode amount 1.5
   post explode grouping blocks
   post explode direction z
   post explode gap 0.05
   post explode mesh on source "Assembly"
   post explode mesh amount 0.2 block "Block_1" source "Assembly"
   post explode offset block "Block_1" 0 0 10 unit "mm" source "Assembly"
   post explode recalculate
   post explode reset
   post properties explosion

Every operation accepts an optional quoted ``view`` name or ID. An omitted
source addresses the stored component scope, or visible sources for a new
explosion. Mesh-on without a source addresses currently visible sources.
``components off`` affects component translations; ``off`` disables both modes.
``reset`` clears settings in the target view. Manual offsets require a source
and unique block name. Amount scales the automatic and manual translations.

Defaults and geometry
---------------------

Explosion starts off. Bare ``post explode`` separates multiple visible sources;
for one source it separates physical blocks; for one block it separates elements.
The automatic axis layout uses a least-squares isotonic packing for each fixed
center order and picks the least squared translation among X/Y/Z with
a gap of 5% of the assembly bounding-box diagonal. Stable piece order breaks
equal-center ties. It retains the arithmetic mean of piece centers. Radial mode
uses center offsets and does not guarantee non-overlapping bounds.

Component amount defaults to 1. Mesh amount defaults to 0.1 and translates each
element by that fraction of its undeformed center's offset from its block center.
Shapes are preserved. Directions and centers do not change during animation.
The slider previews use silent commands; releasing journals the final amount.
Numeric inputs permit amounts from 0 to 100.

Pulse playback
--------------

The explosion properties panel provides Pulse, target, period and minimum/maximum
multiplier controls. Pulse defaults off, with a three-second cosine cycle and
multiplier range 0..1. It starts fully separated, assembles and separates again,
with zero velocity at both extremes. The amount controls remain the static
reference amounts. Targets are components, mesh, or both; disabled explosion
modes stay disabled. Stopping pulse restores the static amounts.

::

   post explode pulse target both
   post explode pulse period 3
   post explode pulse range 0 1
   post explode pulse on
   post explode pulse off

Pulse commands accept an optional quoted ``view`` name or ID and apply to that
view's existing explosion scopes. Period is in seconds (0.1..3600); the range
must satisfy 0 <= minimum <= maximum <= 1. User edits use journaled commands.
The viewport advances pulse once per render with a silent
``post explode pulse advance SECONDS`` command. Result updates take priority
over pulse redraw requests. Shared point buffers are updated once, using cached
displacements. Missed frames are skipped using elapsed time, and camera
interaction remains available. Playback pauses while the workspace is hidden
or animation capture is running. Frame animation and pulse may run together.

Period, range and target are saved with the view. Playback always starts stopped
after reopening a project; runtime phase is not persisted. PNG capture freezes
the current phase. Pulse does not add an automatic time track to MP4 export.

Strict subset/identical element-set aliases follow their containing physical
block. Partially overlapping primary sets are rejected as ambiguous. Mesh
explosion requires original element IDs. Unknown coordinate units can be used
within one source; cross-source assembly layouts require known compatible units.
Native solver coordinates are converted to scene coordinates exactly once.

Deformation is applied first, then native-coordinate clip/section/threshold
filters, then explosion. The original cutting-plane outline is hidden while
exploded because a single plane cannot represent separated element cuts.
Disable explosion to edit that outline. Retained and optional removed geometry,
undeformed outlines, node sets, side sets and integration points follow owners.
Picking records the owning element for a duplicated node. Plot histories still
address the physical entity; viewport labels follow the selected display copy.

Settings are persisted per viewport, with semantic block identities. Geometry
and result arrays are derived, not serialized. Captures, animation export and
report images use the same settings. Missing saved blocks fail visibly rather
than guessing a different block. Reset or select an available block after a
source's structure changes. Polyhedron cells and more than ten million derived
point occurrences per representation are rejected before publishing settings.
