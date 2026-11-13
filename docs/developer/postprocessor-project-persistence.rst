Postprocessor project persistence
=================================

Status: implemented; native Cubit save/open verification is still required.
No build or compilation was performed for this patch.

Storage and ownership
---------------------

The Framework coordinates native project events through
``ProjectStateProviderInterface`` and ``ProjectStateCoordinator``. Each module
owns its serializer. CalculiX remains below ``cubit-sim/workspaces/calculix``,
Report below ``cubit-sim/report``, and Postprocessor uses
``cubit-sim/workspaces/postprocessor``. Existing sibling groups are unchanged.

Postprocessor schema 1 stores a UTF-8 JSON definition payload and schema version
inside HDF5. Payloads are limited to 64 MiB; individual record arrays are bounded
at 100,000 entries. A staged group is flushed before publication. The previous
group is retained during replacement and recovered if the committed group is
absent. This protects the Postprocessor group; it is not an atomic transaction
covering Cubit's entire file or every workspace.

Saved definitions
-----------------

* Source IDs, names and provider/job/file references, with dataset catalogues for
  unavailable-source display. File paths inside the project directory are stored
  relatively; other paths remain absolute. Paths from another operating system
  remain references and cannot be silently interpreted as local relative paths.
* Views, logical split layout, active source/view/representation and camera pose,
  projection and scale. Saved cameras are restored without an automatic fit.
* Per-view source visibility, hidden datasets, frame identity, field/component,
  palette, contours, legend, range mode/manual bounds, independent limit values,
  colors and enabled flags, deformation settings and undeformed overlay.
* Open native table or spatial field table in the side pane, frame identity and pick mode.
  Table rows retain source order; sorting remains deferred.
* Animation rate and loop preference; playback always restores paused.

Frame bindings use step, increment, analysis, mode, subcase, nodal diameter and
physical time. Time agreement uses a relative tolerance of 1e-12 against a scale
of at least one. There is no nearest-frame fallback. Dataset bindings use kind
and model ID where available, otherwise kind and exact name. Ambiguous bindings
make the source unavailable rather than selecting an arbitrary dataset index.

Actors, VTK arrays/pointers, caches, hover, current picks, gesture state, pending
fit operations and revision counters are not persisted. Runtime revisions are
reseeded when publishing a restored session. Machine preferences and defaults
remain in the options system. Qt dock positions are not serialized by this
Postprocessor handler; logical view layout is distinct from native dock layout.

Result identities and arrays
----------------------------

Postprocessor does not duplicate result arrays. CalculiX restoration reuses its
own persisted readers, subject to its existing save-loaded-results option.
A versioned SHA-256 identity covers retained FRD/DAT values, native DAT tables,
resolved mesh/group associations and conversion choices affecting the results.
It excludes paths, worker counts, timestamps and ephemeral conversion run tokens.
Integers have canonical little-endian encoding; floating values use IEEE-754
binary64 with canonical NaN and signed-zero handling. The fingerprint contract
is implemented in ``CoreResultsIdentity`` and must be versioned if changed.

The durable identity is saved alongside CalculiX's retained result state.
Restoration verifies it before conversion and again on publication. It never
re-reads the newest solver output as a substitute for the saved result. If the
required retained arrays are not saved, the Postprocessor source stays
unavailable. External result sources store a versioned SHA-256 bundle identity. The hash
covers content digests in deterministic dependency traversal order, with canonical
paths used for deduplication but excluded from the hash to permit relocation.
External CalculiX bundles include selected FRD/DAT/INP and parsed INP includes.
VTPC bundles recursively include referenced XML collection and leaf files.
Legacy single-file VTT SHA-256 identities remain readable. All dependencies are
verified before loading arrays and checked again before publication.
Missing, modified, unsupported or foreign-platform references retain their names,
catalogues and display settings and show an unavailable reason in the tree.

Read and write lifecycle
------------------------

The coordinator calls every module's definition preparation first, then restores
solver model data, resolves source references, and commits prepared sessions.
CalculiX and Report observers suppress their own duplicate file callbacks while
coordinated. Native reset behavior is retained. Postprocessor commits its session
and runtime registry together after validation; GUI widgets are refreshed after
publication. No arbitrary journal commands are replayed. Ordering does not imply
cross-workspace rollback if a later participant fails after solver restoration.

On save, all modules prepare state before any module writes its own HDF5 group.
A failure reports that Cubit saved the file but workspace persistence was not
fully completed. Temporary native saves are skipped, matching the existing
CalculiX/Report policy; autosave recovery needs a separately validated contract.
``Hdf5FileTransaction`` is for user options and is not used for native .cub files.

Implementation and verification
--------------------------------

* ``Framework/Core/ProjectStateCoordinator`` orders lifecycle phases.
* ``PostprocessorProjectInterface`` resolves providers through Framework APIs.
* ``PostprocessorCorePersistence`` captures, binds and publishes detached state.
* ``PostprocessorSessionCodec`` validates and converts persistent definitions.
* ``PostprocessorProjectPersistence`` owns the staged HDF5 group.

Added verification cases cover definition round-trip, paused playback, preserved
cameras, atomic category toggles, partial category checkboxes, unavailable-source
retention, malformed definitions, VTT identity checks and retained-reader digest
stability. These cases have not been compiled or executed for this patch.

Native Windows and Linux verification must save/open a project with persisted
CalculiX results and a VTT source, confirm camera/frame/display restoration, then
repeat with a missing or modified VTT and with result-array saving disabled.
Also verify opening a legacy project, reset, and a subsequent save of unavailable
sources. Allocation, HDF5 I/O failure and interrupted group replacement require
native integration checks before claiming recovery guarantees.


All-frame table presentation
-----------------------------

The existing table reference identifies a logical history and a zero-based active
frame marker. Row indexes rebuild from restored immutable source snapshots;
combined history rows are not persisted. Older saved single-frame table views
now display all matching frames and highlight their previously saved frame.
Global/step-summary tables keep their own scope. Histories exceeding the current
Qt row limit display an unavailable status on restoration instead of truncating.

Annotation extension
--------------------

Each saved view now has an optional ``annotations`` object. Schema-1 projects
without it remain readable and receive Core defaults. Definitions contain
case-sensitive names, original entity IDs encoded as decimal strings, fixed
field/component selections, symbolic dataset identities (kind, model ID/name),
visibility, normalized text positions, and extrema settings/positions. No result
arrays or calculated extrema are copied. Dataset ordinals are resolved again
against loaded source catalogues, and unavailable definitions retain their
settings. The view annotation revision is reseeded during session publication.
Explicit source removal deletes its labels; source refresh retains definitions.

Plot extension
--------------

An optional plots array and active_plot ID store definitions. Views have optional
plot IDs and a plot_active side-pane flag. Missing fields in older schema-1
projects receive empty defaults. Curves save original IDs as decimal strings,
durable dataset identities, fixed fields/components/reductions or native table,
column and exact filter references. Visibility and optional linear bounds are
saved; scalar buffers and runtime revisions are excluded. Source restoration
still occurs after solver workspace loading. Unavailable references retain their
settings. Definitions are validated before publication, cache revisions reseeded
and restored cameras left intact.
