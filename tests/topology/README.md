# Topology verification

Status: **native checks not executed for this implementation**. Source and Python
syntax checks do not establish a native pass. No build, solver run or GUI test is
performed by adding these files.

## Automated configuration and persistence checks (batch 8a)

Use the plugin and Python wrapper containing
`CalculiXPythonInterface.get_topology_settings()`. The normal CMake SWIG command
already depends on the interface header; regenerate/build it only when a build
is authorized. The script checks wrapper capabilities before resetting the model.
No additional Python package, test framework or solver is needed.

The entry points are:

- `examples/calculix/topology/verification/verify_topology.jou` — the single
  suite entry point, with a manifest covering available and blocked stages.
- `examples/calculix/topology/verification/settings_and_persistence.jou`
- `examples/calculix/topology/verification/settings_and_persistence.py`

Set Cubit's working directory to the repository root or the verification
directory, then play **verify_topology.jou**. It invokes the configuration script
and writes `verification.csv` in a unique suite evidence directory. Native math
stages are BLOCKED: their C++ helpers are not exposed by the present Python API,
and no production changes were authorized in the documentation-only pass.
GUI, historical iteration and solver stages are separately NOT EXECUTED. The
overall manifest therefore remains INCOMPLETE even if configuration passes.
The focused `settings_and_persistence.jou` can still be used independently.

The configuration journal resets the current
model. It creates one unmeshed brick solely to write a native `.cub5` file.
It does not mesh, export a solver input or run a solver. Expected invalid commands
print Cubit errors; a handled expected rejection is not a failed check.

Set `PLUGIN_REVISION` in the journal to the revision of the loaded plugin. A
source checkout revision alone is not proof of the loaded binary's revision.
For direct Python use inside Cubit:

```python
import runpy

suite = runpy.run_path("examples/calculix/topology/verification/settings_and_persistence.py")
output = suite["run_suite"](plugin_revision="<loaded plugin revision>")
```

Each invocation creates a unique directory beneath
`<working directory>/topology_verification/`. It never overwrites a previous
verification run. Evidence consists of:

| File | Contents |
| --- | --- |
| `metadata.csv` | Cubit/Python/platform information, wrapper path and user-supplied plugin revision |
| `commands.csv` | Commands, expected outcome, actual return and error counts |
| `checks.csv` | Individual checks and actual pass/fail outcomes |
| `settings.csv` | Complete queried settings at each comparison point |
| `summary.csv` | Automated result and separate unexecuted coverage categories |
| `settings_and_persistence.cub5` | Density case 1 and level-set case 2 for follow-up GUI checks |
| `failure.txt` | Traceback when a check fails |

The suite stops on the first unexpected result and retains the current model and
earlier evidence. Command processing can fail after partially modifying a model;
the script does not automatically reset or roll back a failed case. Playback
pause-on-error is temporarily disabled around expected rejections and restored
in `finally`. Error counts are recorded without clearing them, and may decrease
across a model reset.

Coverage includes defaults, all eight create/modify level-set parameters,
isolated partial edits, explicit automatic-length zeros, method switches,
rejection without committing an unrelated name edit, refinement integer checks,
dormant settings validation on density cases, the public level-set run gate,
delete/recreate defaults and unrun case save/reset/reopen.

The script compares every setting by keyword. The C++ query returns common and
both methods' settings through Core interfaces. Floating-point strings use the
classic locale and round-trip precision. Defaults in the script are regression
expectations; they are not used to initialize production cases.

## Manual GUI checks

These checks remain **NOT EXECUTED** until an operator records results. Start with
the successful suite's current model or open its saved `.cub5` file. Create
`gui-checks.csv` in the same evidence directory with columns
`check,status,expected,observed,plugin_revision,screenshot`; use PASS, FAIL or
NOT EXECUTED explicitly. Use `dict(ccx.get_topology_settings(id))` to confirm
stored values after GUI actions:

```python
import calculix_pythoninterface

ccx = calculix_pythoninterface.CalculiXPythonInterface()
print(dict(ccx.get_topology_settings(2)))
```

| Check | Procedure and expected outcome | Initial status |
| --- | --- | --- |
| Navigation create | Open Topology > Create > Density-based / Level set. Each opens its complete independent form with the correct method. | NOT EXECUTED |
| Navigation modify | Open each method's Modify panel and select a case. Applying a method conversion submits the corresponding method. | NOT EXECUTED |
| Tree context routing | Modify settings on case 1 opens Density-based; on case 2 it opens Level set. Root Create topology has both method entries. | NOT EXECUTED |
| Tree double-click | Double-click each case. Management selects that case and its stored Method. No extra management tree node is present. | NOT EXECUTED |
| Case properties | Select each case. Properties show the stored method and only its applicable method settings. Renaming keeps the method. | NOT EXECUTED |
| Method selection | In Management Settings, switch Method without Apply. The complete form changes immediately; queried Core settings remain unchanged. | NOT EXECUTED |
| Apply confirmation | Change a value and Apply. The submitted setting and method match the Core query before edits are cleared. | NOT EXECUTED |
| Draft switching | Enter different edits in both methods, switch methods and cases, then return. Each method/case retains its own draft. | NOT EXECUTED |
| Failed Apply | Enter an invalid width or refinement count. Apply is rejected; stored settings remain unchanged and entered values remain visible. | NOT EXECUTED |
| Reload settings | Edit a field and reload that method's settings. Its draft is discarded and stored values return. | NOT EXECUTED |
| Context Run gate | Case 2 has disabled Run in both the context menu and Management. Density case 1 still offers Run; do not execute it on this unmeshed configuration fixture. | NOT EXECUTED |
| Delete cleanup | Delete case 2. Navigation, Management, properties and monitors must not retain stale selections/results for it. Unrelated case drafts remain. | NOT EXECUTED |
| Reset cleanup | Reset the model while both methods have drafts. All topology fields, drafts, case/iteration lists, properties and monitors clear. | NOT EXECUTED |
| Window size | At 1920x1080, each form scrolls and Apply remains reachable. Check both Windows and Linux. | NOT EXECUTED |

Using a separately prepared density analysis (for example the existing beam
example), also record the following checks. They are not covered by the unmeshed
batch 8a journal and require separate execution:

- Settings and Method selection disable while running and become editable again
  after completion or termination.
- Inspect an existing iteration after editing the current case method. Its
  properties and change label still use its captured historical method.
- Confirm historical jobs, factors, designs and evaluations survive a native
  save/load and remain usable for result queries with their corresponding mesh.
- Verify rerun/delete/reset cleanup with actual completed and interrupted jobs.

## Remaining numerical verification (batch 8b and later)

Keep public level-set execution blocked until independently verified. Remaining
coverage includes C3D4/6/8/10/15/20 interpolation and node ordering, regularized
fractions, finite-difference nodal derivatives, preservation, signed volume
constraints, QP/proposal/acceptance behavior, actual-volume backtracking,
convergence, mixed/curved meshes and performance. No command bypass or automatic
gate removal belongs in the batch 8a journal.

The later OpenPisco reference comparison and review of whether its bundled
source can be removed remain on the documented roadmap.
