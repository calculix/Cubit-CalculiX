# Documentation images

Place screenshots here, grouped by feature when needed. Use readable PNG captures
of the actual GUI; record the plugin/Cubit version and explain selections in the
caption. Do not use screenshots from an older panel as evidence of current UI.

The following SVGs are source-controlled explanatory diagrams, not GUI screenshots:

- `topology-workflow.svg`: compact user iteration overview, including the adjoint bypass.
- `topology-run-lifecycle.svg`: developer controller flow, completion checks and stop handling.
- `topology-data-ownership.svg`: model stores, transient state, GUI preferences and external files.

They use local vector shapes and system fonts, without a remote diagram renderer.
Solid ownership arrows and dashed reference/data-flow arrows in the ownership
figure do not describe C++ include dependencies.

## Beam tutorial capture list

Capture the following from the current GUI while following
`docs/tutorials/topology-beam.rst`. Save the actual PNGs under `docs/images/topology/`
when available; this list is a capture plan, not a claim that the files exist.
Add tutorial figure references only after the corresponding images are present.

| Planned file | Capture and context |
| --- | --- |
| `topology/beam-model.png` | Original HEX8 beam mesh, with the fixed support and rigid-body load region identified. |
| `topology/settings.png` | Case 1: density method, displacement Y objective, nodeset 3, target volume fraction 0.7, maximum 50 iterations. |
| `topology/preservation.png` | Preserved-set management showing the automatic support and rigid-body selections; distinguish set rules from resolved element flags. |
| `topology/run.png` | Topology Management Run tab with state, current physical/adjoint job, completed-iteration displacement and three readable limit-usage bars. |
| `topology/actions.png` | Optional independent Actions panel capture showing equivalent run commands and convergence interpretation. |
| `topology/results.png` | Topology Management showing the displayed iteration, navigation, automatic preview, playback, and threshold controls. |
| `topology/threshold-preview.png` | A later completed iteration with green retained elements and red removal edges at the recorded threshold, before mesh deletion. |
| `topology/toolbar.png` | Single-row topology toolbar: case/iteration navigation, threshold icon/field, preview toggle and final CGAL wrap icon. |
| `topology/gif.png` | GIF tab with threshold, playback/export FPS, filename, overwrite and cancellation controls. |
| `topology/alpha-wrap.png` | Alpha wrap tab with displayed iteration/threshold, automatic alpha/offset fields, import and overwrite choices. |
| `topology/jobs-monitor.png` | Jobs Monitor for an actual physical/adjoint job, with its ID identified; ParaView remains available here. |

Record the actual Cubit version, plugin version or revision, case name/ID,
iteration where applicable, threshold for previews, and run state with each
capture. Use those facts in the tutorial figure captions. Report the actual
iteration and convergence state rather than forcing a predetermined outcome.
Keep panel text readable and use a consistent camera for comparable mesh views.
Capture comparisons and exports before deleting elements. Do not use older
screenshots or fabricated panel images to fill the planned filenames. Capture
a case where the three bars pass but the ordinary proposal prevents convergence
only if that state is actually observed; do not manufacture a screenshot or
change result values to illustrate it. Screenshot figure references remain
pending until the corresponding current images exist.

## Toolbar icon candidates

`toolbar-size-icon-candidates/` and `toolbar-threshold-icon-candidates/` each
contain five numbered 24-pixel SVG candidates and a `candidates.svg` comparison
sheet with enlarged and native-size samples. The selected assets are:

- Size 4, ruler: `src/CalculiX/Icons/CCXDrawingSize.svg`.
- Threshold 5, filter limit: `src/CalculiX/Icons/CCXTopologyThreshold.svg`.

The numbered candidate directories remain design alternatives; the selected
assets above are the ones used by the current toolbar code.
