# Adding a feature: follow the existing surface-traction implementation

[Developer documentation](README.md)

Use `loadssurfacetraction` as an implemented example when adding a similar load. This guide traces existing code from user input through model storage, step assignment, persistence, GUI, and export. It does not propose or implement a new command.

## 1. Start with the existing user workflow

[journal_files/surfacetraction.jou](../journal_files/surfacetraction.jou) builds two meshed bricks, defines triangular and quadrilateral sidesets, creates loads, assigns them to a static step, and exports an input deck. Its relevant commands are:

```text
ccx create surfacetraction sideset 1 force_dof_1 0 force_dof_2 1 force_dof_3 0 name "top_quad"
ccx create surfacetraction sideset 2 force_dof_1 0 force_dof_2 1 force_dof_3 0 name "top_tri"
ccx create step name "static" static
ccx step 1 add load surfacetraction 1 2
export ccx "surfacetraction.inp" overwrite
```

This excerpt requires the surrounding model setup; it is not a standalone journal. The full journal resets the model, imports a named library material, and writes into the current directory. Run it in a disposable session/output directory. The higher-order element settings in that journal are commented out, and the journal exports but does not run the solver.

**Implementation semantics:** `force_dof_1/2/3` are distributed total force components. The exporter divides each component among faces in proportion to face area and then among their nodes. It writes `*CLOAD` entries for DOFs 1–3; it does not write a surface-pressure keyword or multiply the input by total area. This follows the formula in `get_load_export()`, not merely the feature's name.

## 2. Trace command registration and input packing

The three command implementations are:

- [ccxLoadsSurfaceTractionCreateCommand.cpp](../src/Commands/ccxLoadsSurfaceTractionCreateCommand.cpp)
- [ccxLoadsSurfaceTractionModifyCommand.cpp](../src/Commands/ccxLoadsSurfaceTractionModifyCommand.cpp)
- [ccxLoadsSurfaceTractionDeleteCommand.cpp](../src/Commands/ccxLoadsSurfaceTractionDeleteCommand.cpp)

Each provides syntax/help and an `execute()` entry point. [CalculiXPlugin.cpp](../src/CalculiXPlugin.cpp) includes their headers, advertises keys in `get_keys()`, and constructs them in `create_command()`. [Commands/CMakeLists.txt](../src/Commands/CMakeLists.txt) explicitly includes their source/header files.

Creation requires a sideset and all three force components. Optional inputs are `op`, `amplitude`, `timedelay`, and `name`. The create command packs these arguments as follows:

| Argument vector | Contents in index order |
| --- | --- |
| `options` (strings) | OP mode, amplitude ID, time delay, sideset ID, name |
| `options2` (doubles) | Force component 1, component 2, component 3 |

Defaults are OP mode `0` (MOD), amplitude ID `-1`, empty delay, and empty name. NEW is stored as `1`. Modification additionally passes eight presence markers in the order OP, amplitude, delay, sideset, force 1, force 2, force 3, name. A missing component is preserved rather than set to the placeholder zero packed by the command.

[CalculiXCoreInterface.cpp](../src/Core/CalculiXCoreInterface.cpp), `create_loadssurfacetraction()`, `modify_loadssurfacetraction()`, and `delete_loadssurfacetraction()`, forwards to the shared core. Matching methods in [CalculiXCore.cpp](../src/Core/CalculiXCore.cpp) forward to `CoreLoadsSurfaceTraction`.

**For a similar new feature:** wire all three registration locations, the source lists, and both forwarding layers. Specify vector layouts explicitly so commands and core methods agree.

## 3. Understand the stored data and lifecycle

[CoreLoadsSurfaceTraction.hpp](../src/Core/CoreLoadsSurfaceTraction.hpp) declares four tables. [CoreLoadsSurfaceTraction.cpp](../src/Core/CoreLoadsSurfaceTraction.cpp) implements their creation, lookup, modification, and deletion.

| Table | Row layout |
| --- | --- |
| `loads_data` (`int`) | load ID, OP mode, amplitude ID, time-delay ID, sideset ID, force ID, name ID |
| `time_delay_data` (`string`) | time-delay ID, value |
| `force_data` (`double`) | force ID, component 1, component 2, component 3 |
| `name_data` (`string`) | name ID, name |

IDs are references, not vector indices. Use `get_*_data_id_from_*_id()` lookup methods before accessing rows. `create_load()` chooses ID 1 for an empty table, otherwise the last row's ID plus one, and creates associated delay/force/name rows. `delete_load()` removes those associated rows and the load row. `reset()` clears all four tables.

`CalculiXCore` owns the object: constructor pointer initialization, allocation and `init()` in core initialization, reset, destructor deletion, and `print_data()` integration are all present. The feature's own `update()` is a no-op returning true; reference cleanup is handled elsewhere in the central core.

**For a similar new feature:** implement these lifecycle paths together. Do not assume a successful create method validates geometry or referenced IDs; this implementation stores the supplied sideset/amplitude IDs directly.

## 4. Preserve model files and references

`CalculiXCore::read_cub()` and `save_cub()` read/write all four tables under:

```text
Cubit-CalculiX/Loads/SurfaceTraction
```

The dataset names match the table names. Load/step associations are persisted separately as `loads_data` under `Cubit-CalculiX/Steps`.

The central core's cleanup logic clears missing amplitude references to `-1`, deletes surface-traction objects whose sidesets no longer exist, and removes stale surface-traction references from steps. Cleanup execution depends on the surrounding update/settings path; its timing was not runtime-tested here.

**For a similar new feature:** add both persistence directions and define behavior for older files missing the new datasets. Cover cleanup of geometry, optional references, and step membership. A new table layout requires compatibility decisions, not just a writer change.

## 5. Connect the load to analysis steps

[ccxStepAddLoadCommand.cpp](../src/Commands/ccxStepAddLoadCommand.cpp) maps `surfacetraction` to load type **9** and calls `step_add_loads()`. [ccxStepRemoveLoadCommand.cpp](../src/Commands/ccxStepRemoveLoadCommand.cpp) uses the same type for removal. [CoreSteps.cpp](../src/Core/CoreSteps.cpp), `add_loads()`, stores membership and avoids an existing duplicate association.

Step export in `CalculiXCore.cpp` matches type 9 and the load ID before calling `get_load_export()`. Merely creating the load does not assign it to a step. The core wraps its export with custom lines identified by `BEFORE`/`AFTER`, `SURFACETRACTION`, and the load ID; [ccxCustomLineCreateCommand.cpp](../src/Commands/ccxCustomLineCreateCommand.cpp) and the modify command recognize that keyword.

**For a similar new feature:** select an unused type and update all relevant dispatch sites.

## 6. Follow export down to nodal forces

`CoreLoadsSurfaceTraction::get_load_export()`:

1. Starts `*CLOAD`, emits `OP=NEW` only for mode 1, and appends an amplitude name and time delay when set. MOD is left implicit.
2. Queries sideset nodes, triangles, and faces through `CubitInterface`, and obtains triangle/quad areas.
3. Sums the areas. For each force component and face, computes `component × face_area / total_area`.
4. Calls `calc_consistent_load()` with expanded connectivity, accumulates contributions at shared nodes, and writes nonzero nodal components.

The implemented weights are:

| Face connectivity size | Weights applied to that face's force |
| --- | --- |
| 3-node triangle | `1/3` at each node |
| 6-node triangle | First three nodes: zero; last three: `1/3` each |
| 4-node quadrilateral | `1/4` at each node |
| 8-node quadrilateral | First four: `-1/12` each; last four: `1/3` each |

These are the code's formulas. Each row sums to one, so the expected summed nodal force equals the supplied component when all faces/connectivity are supported, area is valid, and every connectivity node is included. Correctness for curved/distorted higher-order faces and Cubit's connectivity ordering remains **unverified**.

[ccxExportCommand.cpp](../src/Commands/ccxExportCommand.cpp) writes the resulting step output. Unlike a global keyword such as damping, this load is integrated through step export, not a separate top-level `write_surfacetraction()` function.

## 7. Wire the GUI, selection, and drawing

The existing integration points are:

| Concern | Source |
| --- | --- |
| Create/modify/delete widgets | [LoadsSurfaceTractionCreatePanel.cpp](../src/GUI/LoadsSurfaceTractionCreatePanel.cpp), [modify panel](../src/GUI/LoadsSurfaceTractionModifyPanel.cpp), [delete panel](../src/GUI/LoadsSurfaceTractionDeletePanel.cpp) |
| Panel markers and construction | [cmdPanelManager.cpp](../src/cmdPanelManager.cpp), [MyCmdWidgetFactory.cpp](../src/MyCmdWidgetFactory.cpp): `CCXLoadsSurfaceTractionCreate/Modify/Delete` |
| Global and per-step trees | [LoadsSurfaceTractionTree.cpp](../src/GUI/LoadsSurfaceTractionTree.cpp), [StepsLoadsSurfaceTractionTree.cpp](../src/GUI/StepsLoadsSurfaceTractionTree.cpp) |
| Context/navigation and step editing | [ModelTree.cpp](../src/GUI/ModelTree.cpp), [StepsManagement.cpp](../src/GUI/StepsManagement.cpp) |
| Display data, entity resolution, parsing | [CalculiXCore.cpp](../src/Core/CalculiXCore.cpp): search `loadssurfacetraction` and the two `get_*loadssurfacetraction_tree_data()` methods |
| Load drawing | [CoreDraw.cpp](../src/Core/CoreDraw.cpp), plus sideset coordinates and force-component preparation in the central core |

The create panel builds `ccx create surfacetraction ...` and submits it through `GuiSendCmd::gui_cmd()`. Step management constructs add/remove-load commands. New GUI files also belong in [GUI/CMakeLists.txt](../src/GUI/CMakeLists.txt); core files belong in [Core/CMakeLists.txt](../src/Core/CMakeLists.txt).

No dedicated surface-traction method was found in the current `CalculiXPythonInterface.hpp`. Do not infer that every command has its own Python binding.
