# Source-code map

[Developer documentation](README.md)

## Where to start

| Responsibility | Source location | Useful entry points or contents |
| --- | --- | --- |
| Command registration | [CalculiXCommands.cpp](../src/CalculiX/Commands/CalculiXCommands.cpp) | `command_keys()`, `create_command()`; framework exposes Cubit entry points |
| Workspace switching | [Framework GUI](../src/Framework/GUI/), [WorkspaceCommand.cpp](../src/Framework/Commands/WorkspaceCommand.cpp) | Command-driven Cubit/CalculiX workspaces; [Stage 3](developer/framework-stage3.rst) |
| Individual command syntax and execution | [Commands](../src/CalculiX/Commands/) | `ccx*Command.hpp/.cpp`; `get_syntax()`, `get_syntax_help()`, `get_help()`, `execute()` |
| GUI component lifecycle | [FEAComp.cpp](../src/Framework/FEAComp.cpp), [CalculiXGui.cpp](../src/CalculiX/GUI/CalculiXGui.cpp) | `start_up()`, `clean_up()`, internal GUI `start()`/`stop()` and `update()` |
| Shared model instance | [loadCalculiXCore.cpp](../src/CalculiX/Core/loadCalculiXCore.cpp) | Global `ccx_core` |
| Interface used by commands, GUI, and Python | [Domain interface layout](developer/interface-layout.rst) | Forwarders, model queries, export access |
| Model coordination and persistence | [CalculiXCore.cpp](../src/CalculiX/Core/CalculiXCore.cpp) | `init()`, `init2()`, `update()`, `reset()`, `read_cub()`, `save_cub()` |
| Feature state and behavior | [Core](../src/CalculiX/Core/) | `CoreMaterials`, `CoreSections`, `CoreLoads*`, `CoreBCs*`, `CoreSteps`, `CoreCustomLines`, and others |
| Input-deck assembly | [ccxExportCommand.cpp](../src/CalculiX/Commands/export/ccxExportCommand.cpp) | `write_*()` methods and ordered export orchestration |
| Solver jobs | [CoreJobs.cpp](../src/CalculiX/Core/jobs/CoreJobs.cpp) | `run_job()`, `wait_job()`, `kill_job()`, `check_jobs()` |
| Results | [CoreResults.cpp](../src/CalculiX/Core/results/CoreResults.cpp) | `load_result()`, `convert_result()`; adjacent FRD/DAT readers, VTK writer, and projection code |
| GUI trees and panels | [GUI](../src/CalculiX/GUI/) | `ModelTree.cpp`, `*Tree.cpp`, `*Panel.cpp`, material-management widgets |
| Panel registration/navigation | [cmdPanelManager.cpp](../src/CalculiX/GUI/cmdPanelManager.cpp), [MyCmdWidgetFactory.cpp](../src/CalculiX/GUI/MyCmdWidgetFactory.cpp) | Marker names, navigation nodes, widget creation |
| GUI command submission | [GuiSendCmd.cpp](../src/CalculiX/GUI/application/GuiSendCmd.cpp) | `gui_cmd()` |
| Host events | [Observer.cpp](../src/CalculiX/EventObservers/Observer.cpp) | Command completion, reset, file read/save notifications |
| Python API and binding declarations | [PythonInterface](../src/CalculiX/PythonInterface/) | `CalculiXPythonInterface.hpp/.cpp` and `.i` |
| Settings | [UserOptions](../src/CalculiX/UserOptions/) | Options, configuration, settings panels |
| Utilities | [Utility](../src/CalculiX/Utility/) | `HDF5Tool`, `PlotChart`, `StopWatch`, `ThreadPool`; also bundled dependencies |
| Build definition | [src/CMakeLists.txt](../src/CMakeLists.txt) | Dependency discovery, subdirectories, library targets, SWIG generation |
| Usage examples | [examples](../examples/), [journal_files](../journal_files/) | Journal/Python scripts and model assets; not assumed to be automated regression tests |

## Build and generated files

The root CMake file discovers shared dependencies and adds CalculiX and Framework. `CalculiX/CMakeLists.txt` defines the internal module, command and GUI libraries, the shared core/options and Python API. Framework defines the only Cubit entry points: `fea_plugin` and `fea_comp`. See [Stage 2](developer/framework-stage2.rst). Subdirectory CMake files explicitly enumerate sources; adding a file generally requires editing the corresponding list.

SWIG interfaces are [fea_comp.i](../src/Framework/fea_comp.i) and [CalculiXPythonInterface.i](../src/CalculiX/PythonInterface/CalculiXPythonInterface.i). The latter declares vector/pair templates and includes the Python API header. Treat generated `*_wrap.cxx` files and Python proxy files as outputs, and make API changes in the headers, implementations, and `.i` files first.

`src/Solvers`, `src/Postprocessor`, `src/Library`, and portions of `src/Utility` contain bundled tools, libraries, or third-party sources.
