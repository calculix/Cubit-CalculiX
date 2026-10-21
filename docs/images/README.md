# Documentation images

Place screenshots here, grouped by feature when needed. Use readable PNG captures
of the actual GUI; record the plugin/Cubit version and explain selections in the
caption. Do not use screenshots from an older panel as evidence of current UI.

`topology-workflow.svg` is a source-controlled explanatory diagram, not a GUI screenshot.

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
| `topology/actions.png` | Actions panel with run state, a completed iteration, displacement measurement, and readable convergence indicators. |
| `topology/results.png` | Results panel showing the displayed iteration, navigation, automatic preview, playback, and threshold controls. |
| `topology/threshold-preview.png` | A later completed iteration with green retained elements and red removal edges at the recorded threshold. |

Record the actual Cubit version, plugin version or revision, case name/ID,
iteration where applicable, threshold for previews, and run state with each
capture. Use those facts in the tutorial figure captions. Report the actual
iteration and convergence state rather than forcing a predetermined outcome.
Keep panel text readable and use a consistent camera for comparable mesh views.
Capture comparisons and exports before deleting elements. Do not use older
screenshots or fabricated panel images to fill the planned filenames.
