CalculiX job launch archives
============================

Jobs retain their latest successfully launched run as an immutable
``CalculiXJobRunRecord``. Its launch ID is independent of resource-monitor IDs
and VTK result-conversion tokens. Reporting uses recorded input and supported
frozen setup scenes instead of reconstructing a job from today's model.

Preparation and launch
----------------------

The existing job-export context carries the destination filename. After writing
the input's steps, export flushes the file and calls its snapshot hook before
cleaning up prepared export data. Communication uses ``CalculiXExportInterface``,
``CalculiXJobsInterface`` and ``CalculiXDrawInterface``. The hook never renders,
submits a graphics frame or dispatches another command.

``CoreJobSnapshotBuilder`` reads the emitted node/connectivity records and freezes
prepared step scenes. Unexported live mesh is not used as the job mesh. High-order
faces use corner polygons, matching normal captures. Symbol size is 10% of the
largest exported mesh extent in world coordinates. Emitted step count must match
the frozen model/export order.

Before launch, Jobs archives the final root input and supported INCLUDE closure
again. Only an identical manifest retains the prepared setup snapshot. The hash
covers root/file identities, traversal order, dependency edges and SHA-256 file
hashes. Exact input bytes are retained without rewriting the deck. This records
files observed at launch, not protection against external edits during a run.

Both Windows and Linux publish only after successful process startup. Failed
startup leaves the previous launched record intact. A new launch without usable
setup replaces the old association with an explicit unavailable record. Snapshot
failure does not abort solver startup. Imported input copying uses checked atomic
Qt file output on both platforms.

Coverage
---------

Ordinary generated jobs can freeze supported emitted mesh and typed conditions.
Imported jobs can archive input without a typed setup snapshot. Custom input,
unsupported elements and transformed/generated mesh, restart or model-change
constructs do not receive false setup verification. Moving trajectory scenes
remain unavailable. Each scene records its own coverage reason.

Topology and adjoint contexts retain an unavailable marker and skip heavy input
and setup archiving. No full topology run-history feature is introduced. Jobs
without an archived launch retain unverified external-deck reporting. A newer
unavailable launch never uses an older setup snapshot.

Only step-assigned conditions are frozen. Unassigned model definitions are not
substituted for conditions in the run. A job without analysis steps can therefore
have a model overview but no verified combined setup image.

Persistence
------------

Archive schema 1 is stored at
``CalculiXModelSchema::path("Jobs/RunSnapshots")`` in project HDF5 data. Each job
has launch metadata, exact input-byte datasets, a dependency manifest, one model
mesh and per-step geometry and label metadata.

Geometry uses numeric points, offsets, primitive kinds and annotation indices.
Colors/text use a palette. Dataset shapes and limits are checked before reading.
Per-record limits include 128 MiB/4096 input files, 256 steps, one million
primitives per geometry, 24 million coordinate scalars per snapshot and 16 MiB
per JSON metadata dataset.

Input/setup fingerprints are checked on restore. Reads build a candidate map and
commit with other job data only after job references validate. Writes stage an
archive group before replacing the active group. An interrupted commit with a
missing active group is an error, not a reason to substitute live model data.
Reset/delete clear associated launch records and transient preparation state.

Reporting
----------

``ReportSource`` carries ``solver_launch_id`` and ``setup_snapshot_id`` separately
from ``run_identity``. Job discovery can use archived input when original paths
no longer exist. The input reader resolves INCLUDEs inside the bundle, with
hash/cycle/depth checks and no filesystem fallback.

The image provider checks launch/snapshot IDs and fingerprint before calling
``render_snapshot``. ``CoreDrawSnapshotCapture`` composes stored geometry and
label definitions without live Cubit mesh or condition queries. Camera,
resolution and labels still use report profile settings. Image provenance records
launch/snapshot identities.

Current-model edits leave archived setup unchanged. A new launch invalidates
ordinary update even if input bytes are identical; explicit generation replaces
the job's report. Saved reports retain their PNGs for offline preview and PDF/ODT.
This feature does not bind conversion tokens or certify result-file lineage.

Verification
-------------

Added regression cases cover INCLUDE archives after source deletion, tampering,
frozen scene/profile changes, missing steps, HDF5 round-trip, corrupt geometry,
launch replacement and unchanged-input rerun detection. Tests were not compiled
or executed while builds remained prohibited.

After builds are permitted, check ordinary and failed launches on both platforms,
imported input, unavailable setup without solver failure, model edits after a run,
project save/load, removed source files, profile changes and reruns. Topology and
adjoint launches must retain their normal behavior with unavailable diagnostics.
