Mesh quality reporting
======================

Report settings / Mesh quality contains sibling pages for Hexahedra,
Tetrahedra, Quadrilaterals and Triangles. Each page has metric inclusion,
editable minimum/maximum limits, explicit limit units and Restore family defaults. Empty bounds mean
unbounded. Apply submits readable report commands; Cancel discards pending edits.
Selections and limits persist with report profiles and project report settings.

These are reporting defaults, independent of Cubit's live Quality Defaults.
The catalogue contains geometric shape metrics and dimensional length/area/volume metrics. Limits retain explicit units in profiles; values and limits are converted together to the frozen Report presentation units. Unknown native geometry units make dimensional metrics unavailable. Hex limits
follow the supplied Cubit defaults (scaled Jacobian 0.5 to 1.01); the other
families follow Coreform's published metric tables. Native SDK support remains
specific to the family, order and metric. Unsupported queries produce availability
diagnostics, never zero-valued substitutes or automatic passes.

The content tree controls Mesh quality / Per-block statistics, Quality images,
Failing-element images and Detailed element values. Statistics and images are
on by default; detailed element listings are off and no listing rows are built
unless explicitly enabled. Scaled Jacobian is initially selected in each family.

For example::

   report configure settings option "mesh.quality.hex.scaled_jacobian.minimum" value "0.5" option "mesh.quality.hex.scaled_jacobian.maximum" value "1.01"
   report configure settings option "mesh.quality.element_values" value "false"

Statistics and numerical scope
------------------------------

Each block is partitioned by native element family and nodes per element.
Statistics include element counts, evaluated and unevaluable samples, extrema
with original global element IDs, mean, population standard deviation,
applied limits and failing count/percentage. Only finite samples participate;
percentages use evaluated elements as their denominator. Equal extrema select
the smallest original element ID. Bounds are inclusive. No bounds means not
assessed, rather than pass. The population standard deviation is sqrt(M2/n),
computed with Welford's algorithm.

Pointwise native queries use chunks of 2048 elements. Reference-dependent
metrics are submitted in one complete block/family/order query and do not
change Cubit's reference settings. They therefore retain native reference
semantics; their call cannot be interrupted internally. Arrays and identities
are retained in an immutable snapshot. Progress/cancellation is checked between
queries, detail batches, image composition batches and render tasks.

Images
------

Images are generated through a private Postprocessor capture application. They
show native measured cell values on undeformed corner topology, with native
higher-order metric evaluation independent of that display simplification.
The caption records the family and native node count. Quality values are not
recomputed from the linearized image geometry.

Quality captures use the selected Postprocessor view, camera/background choices
and applicable per-block appearance. Their scalar coloring is quality-specific;
result vectors, deformation, explosion and result filters are absent from the
private quality scene. The live Postprocessor remains unchanged. Full HD is the
default. Failing-element images use transparent model context and are generated
only when failures exist. Sections record the exact acceptance limits used.

Job archives and resource limits
--------------------------------

Current-model reports query the current mesh. Job export attempts to capture
catalogue metrics, prioritizing the initially enabled metrics. Quality is bound
to the archived input manifest only after original node IDs, ordered element
connectivity, exported block membership and coordinates are verified. Coordinate
comparison allows the export formatting tolerance of 1e-8 times max(1,abs(x)) in
recorded native geometry units. Unsupported exported permutations cause an
availability diagnostic; they are not guessed. An unavailable quality capture
does not prevent the solver export itself. Native archive collection has a
scoped Cubit busy progress bar with interrupt checks; normal report generation
continues to use its existing progress scope through preview readiness.

Job reports use only this recorded quality payload, independently of the live
mesh. Changing report thresholds reclassifies the archived native values and
stores the applied limits in the resulting report. Missing archived metrics
remain unavailable. Binary payloads have a SHA-256 checksum and bounded counts;
quality data is retained inside the existing .cub job archive, not external files.

The capture limits are two million elements, eight million nodes and a 240-MiB
estimated numeric payload. Optional metrics exceeding the remaining budget are
marked unavailable. Serialized archives are limited to 256 MiB. Geometry and
values are retained once per job capture; image composition shares coordinates
and processes individual metric images. PDF and ODT use the existing table and
image export paths.

Catalogue references
--------------------

* https://coreform.com/cubit_help/latest/mesh_generation/mesh_quality_assessment/hexahedral_metrics.htm
* https://coreform.com/cubit_help/latest/mesh_generation/mesh_quality_assessment/tetrahedral_metrics.htm
* https://coreform.com/cubit_help/latest/mesh_generation/mesh_quality_assessment/quadrilateral_metrics.htm
* https://coreform.com/cubit_help/latest/mesh_generation/mesh_quality_assessment/triangular_metrics.htm
