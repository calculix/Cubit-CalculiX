# Source-code map

[Developer documentation](README.md)

The [project-code guide](developer/code-documentation.rst) supplies ownership,
state, error and extension contracts for all project domains. The
[complete owned-source catalogue](developer/code-catalog/index.rst) links the
current Framework and CalculiX sources, including their maintained build files.
It excludes external utility implementations, binary libraries and solver bundles.
The catalogue is source-navigation coverage, not a statement that every branch
has been executed or every declaration has a complete semantic audit.

## Where to start

| Responsibility | Source location | Useful entry points or contents |
| --- | --- | --- |
| Command registration | [CalculiXCommands.cpp](../src/CalculiX/Commands/CalculiXCommands.cpp) | `command_keys()`, `create_command()`; framework exposes Cubit entry points |
| Workspace switching | [Framework GUI](../src/Framework/GUI/), [WorkspaceCommand.cpp](../src/Framework/Commands/WorkspaceCommand.cpp) | Command-driven Cubit/CalculiX workspaces; [Stage 3](developer/framework-stage3.rst) |
| Individual command syntax and execution | [Commands](../src/CalculiX/Commands/) | `ccx*Command.hpp/.cpp`; `get_syntax()`, `get_syntax_help()`, `get_help()`, `execute()` |
| GUI component lifecycle | [CubitSimComp.cpp](../src/Framework/CubitSimComp.cpp), [CalculiXGui.cpp](../src/CalculiX/GUI/CalculiXGui.cpp) | `start_up()`, `clean_up()`, internal GUI `start()`/`stop()` and `update()` |
| Shared model instance | [loadCalculiXCore.cpp](../src/CalculiX/Core/loadCalculiXCore.cpp) | Global `ccx_core` |
| Interface used by commands, GUI, and Python | [Domain interface layout](developer/interface-layout.rst) | Forwarders, model queries, export access |
| Model coordination and persistence | [Core declarations](../src/CalculiX/Core/CalculiXCore.hpp), [application lifecycle](../src/CalculiX/Core/application/CalculiXCoreInitialization.cpp), [maintenance](../src/CalculiX/Core/application/CalculiXCoreMaintenance.cpp), [persistence](../src/CalculiX/Core/application/CalculiXCorePersistence.cpp) | Definitions are split across domain translation units; `init()`, `init2()`, `update()`, `reset()`, `read_cub()`, `save_cub()` |
| Feature state and behavior | [Core](../src/CalculiX/Core/) | `CoreMaterials`, `CoreSections`, `CoreLoads*`, `CoreBCs*`, `CoreSteps`, `CoreCustomLines`, and others |
| Input-deck assembly | [ccxExportCommand.cpp](../src/CalculiX/Commands/export/ccxExportCommand.cpp) | `write_*()` methods and ordered export orchestration |
| Solver jobs | [CoreJobs.cpp](../src/CalculiX/Core/jobs/CoreJobs.cpp) | `run_job()`, `wait_job()`, `kill_job()`, `check_jobs()` |
| Results | [CoreResults.cpp](../src/CalculiX/Core/results/CoreResults.cpp) | `load_result()`, `convert_result()`; adjacent FRD/DAT readers, VTK writer, and projection code |
| Postprocessor sources | [PostprocessorCore.hpp](../src/Postprocessor/Core/application/PostprocessorCore.hpp), [source lifecycle](../src/Postprocessor/Core/sources/PostprocessorCoreSources.cpp) | Open/refresh/select/close, immutable runtime handles; [architecture and contracts](developer/postprocessor.rst), [verification checklist](developer/postprocessor-verification.rst) |
| Postprocessor module and commands | [PostprocessorModule.cpp](../src/Postprocessor/PostprocessorModule.cpp), [application interface](../src/Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp), [command factory](../src/Postprocessor/Commands/PostprocessorCommands.cpp) | Registry injection, weak command lifetimes, [source command reference](postprocessor/commands.rst) |
| Postprocessor workspace | [PostprocessorGui.cpp](../src/Postprocessor/GUI/PostprocessorGui.cpp), [workspace](../src/Postprocessor/GUI/Workspace/PostprocessorWorkspace.cpp), [graphics stack](../src/Framework/GUI/WorkspaceGraphicsStack.cpp) | [GUI integration and ownership](developer/postprocessor-workspace.rst), [workspace guide](postprocessor/workspace.rst); separate empty VTK viewport |
| Topology orchestration | [CoreTopologyOptimization.cpp](../src/CalculiX/Core/topology/CoreTopologyOptimization.cpp), [run coordination](../src/CalculiX/Core/topology/CalculiXCoreTopologyRun.cpp) | Physical/adjoint phases, run/wait/kill and completion; [implementation map](developer/topology.rst) |
| Topology numerical operations | [CoreTopology.cpp](../src/CalculiX/Core/topology/CoreTopology.cpp), [record types](../src/CalculiX/Core/topology/CoreTopologyStructs.hpp) | Densities, objective derivatives, filtering, updates and convergence; [mathematics](developer/topology-algorithms.rst) |
| Topology export and results | [Topology helpers](../src/CalculiX/Core/topology/), [step export](../src/CalculiX/Core/steps/CalculiXCoreStepExport.cpp) | [Export/helper map](developer/topology-export.rst), [preservation map](developer/topology-preservation.rst), [result-consumer map](developer/topology-results.rst) |
| Integration and connectivity | [ElementIntegration.cpp](../src/CalculiX/Utility/ElementIntegration.cpp), [ElementNodeOrdering.hpp](../src/CalculiX/Utility/ElementNodeOrdering.hpp) | CalculiX quadrature and Cubit/Exodus-to-CalculiX local node ordering |
| GUI trees and panels | [GUI](../src/CalculiX/GUI/) | `ModelTree.cpp`, `*Tree.cpp`, `*Panel.cpp`, material-management widgets |
| Panel registration/navigation | [cmdPanelManager.cpp](../src/CalculiX/GUI/cmdPanelManager.cpp), [MyCmdWidgetFactory.cpp](../src/CalculiX/GUI/MyCmdWidgetFactory.cpp) | Marker names, navigation nodes, widget creation |
| GUI command submission | [GuiSendCmd.cpp](../src/CalculiX/GUI/application/GuiSendCmd.cpp) | `gui_cmd()` |
| Host events | [CoreObserver.cpp](../src/CalculiX/Core/application/CoreObserver.cpp), [GUI Observer.cpp](../src/CalculiX/EventObservers/Observer.cpp) | Model command/reset/file notifications and the GUI update callbacks installed by `CalculiXGui` |
| Python API and binding declarations | [PythonInterface](../src/CalculiX/PythonInterface/) | `CalculiXPythonInterface.hpp/.cpp` and `.i` |
| Workspace user options | [CoreUserOptions.cpp](../src/CalculiX/Core/application/CoreUserOptions.cpp), [GUI options](../src/CalculiX/GUI/options/) | Core-owned declarations/defaults and command-driven editing; effective values remain in `ccx_uo` |
| Framework user-options store | [UserOptionsStore.cpp](../src/Framework/Storage/UserOptionsStore.cpp) | One HDF5 file, typed values, atomic saves and workspace isolation; [contract](developer/user-options.rst) |
| Shared HDF5 storage | [Framework storage](../src/Framework/Storage/) | `cubitsim_storage` provides `HDF5Tool` to workspace libraries; workspace persistence classes own their schemas |
| Utilities | [Utility](../src/CalculiX/Utility/) | `PlotChart`, `StopWatch`, `ThreadPool`; also bundled dependencies |
| Build definition | [src/CMakeLists.txt](../src/CMakeLists.txt) | Dependency discovery, subdirectories, library targets, SWIG generation |
| Usage examples | [examples/calculix](../examples/calculix/), [journal_files](../journal_files/) | Journal/Python scripts and model assets; not assumed to be automated regression tests |

## Build and generated files

The root CMake file discovers shared dependencies and adds framework storage before CalculiX and the framework host. `Framework/Storage/CMakeLists.txt` defines `cubitsim_storage`, which links HDF5 and Cubit utilities without depending on workspace modules. `CalculiX/CMakeLists.txt` defines the internal module, command and GUI libraries, the shared core/options and Python API. Framework defines the only Cubit entry points: `cubitsim_plugin` and `cubitsim_comp`. See [Stage 2](developer/framework-stage2.rst). Subdirectory CMake files explicitly enumerate sources; adding a file generally requires editing the corresponding list.

SWIG interfaces are [cubitsim_comp.i](../src/Framework/cubitsim_comp.i) and [CalculiXPythonInterface.i](../src/CalculiX/PythonInterface/CalculiXPythonInterface.i). The latter declares vector/pair templates and includes the Python API header. Treat generated `*_wrap.cxx` files and Python proxy files as outputs, and make API changes in the headers, implementations, and `.i` files first.

`src/Solvers`, `src/Library`, and portions of `src/Utility` and `src/CalculiX/Utility` contain bundled tools, libraries, or third-party sources. These directories are not blanket API-extraction inputs; `docs/Doxyfile` explicitly selects the documented project utilities.
