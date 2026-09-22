# Source-code map

[Developer documentation](README.md)

## Where to start

| Responsibility | Source location | Useful entry points or contents |
| --- | --- | --- |
| Command registration | [CalculiXPlugin.cpp](../src/CalculiXPlugin.cpp) | `get_keys()`, `create_command()` |
| Individual command syntax and execution | [Commands](../src/Commands/) | `ccx*Command.hpp/.cpp`; `get_syntax()`, `get_syntax_help()`, `get_help()`, `execute()` |
| GUI component lifecycle | [CalculiXComp.cpp](../src/CalculiXComp.cpp), [loadCalculiXComp.cpp](../src/loadCalculiXComp.cpp) | `start_up()`, `clean_up()`, `update()`, plugin registration |
| Shared model instance | [loadCalculiXCore.cpp](../src/Core/loadCalculiXCore.cpp) | Global `ccx_core` |
| Interface used by commands, GUI, and Python | [CalculiXCoreInterface.hpp](../src/Core/CalculiXCoreInterface.hpp), [implementation](../src/Core/CalculiXCoreInterface.cpp) | Forwarders, model queries, export access |
| Model coordination and persistence | [CalculiXCore.cpp](../src/Core/CalculiXCore.cpp) | `init()`, `init2()`, `update()`, `reset()`, `read_cub()`, `save_cub()` |
| Feature state and behavior | [Core](../src/Core/) | `CoreMaterials`, `CoreSections`, `CoreLoads*`, `CoreBCs*`, `CoreSteps`, `CoreCustomLines`, and others |
| Input-deck assembly | [ccxExportCommand.cpp](../src/Commands/ccxExportCommand.cpp) | `write_*()` methods and ordered export orchestration |
| Solver jobs | [CoreJobs.cpp](../src/Core/CoreJobs.cpp) | `run_job()`, `wait_job()`, `kill_job()`, `check_jobs()` |
| Results | [CoreResults.cpp](../src/Core/CoreResults.cpp) | `load_result()`, `convert_result()`; adjacent FRD/DAT readers, VTK writer, and projection code |
| GUI trees and panels | [GUI](../src/GUI/) | `ModelTree.cpp`, `*Tree.cpp`, `*Panel.cpp`, material-management widgets |
| Panel registration/navigation | [cmdPanelManager.cpp](../src/cmdPanelManager.cpp), [MyCmdWidgetFactory.cpp](../src/MyCmdWidgetFactory.cpp) | Marker names, navigation nodes, widget creation |
| GUI command submission | [GuiSendCmd.cpp](../src/GUI/GuiSendCmd.cpp) | `gui_cmd()` |
| Host events | [Observer.cpp](../src/EventObservers/Observer.cpp) | Command completion, reset, file read/save notifications |
| Python API and binding declarations | [PythonInterface](../src/PythonInterface/) | `CalculiXPythonInterface.hpp/.cpp` and `.i` |
| Settings | [UserOptions](../src/UserOptions/) | Options, configuration, settings panels |
| Utilities | [Utility](../src/Utility/) | `HDF5Tool`, `PlotChart`, `StopWatch`, `ThreadPool`; also bundled dependencies |
| Build definition | [src/CMakeLists.txt](../src/CMakeLists.txt) | Dependency discovery, subdirectories, library targets, SWIG generation |
| Usage examples | [examples](../examples/), [journal_files](../journal_files/) | Journal/Python scripts and model assets; not assumed to be automated regression tests |

## Build and generated files

The local top-level CMake file defines `calculix_plugin`, `calculix_core`, `calculix_comp`, `calculix_pythoninterface`, and `calculix_useroptions`. The command, component, and Python targets link to the core. Subdirectory CMake files explicitly enumerate sources; adding a file generally requires editing the corresponding list.

SWIG interfaces are [calculix_comp.i](../src/calculix_comp.i) and [CalculiXPythonInterface.i](../src/PythonInterface/CalculiXPythonInterface.i). The latter declares vector/pair templates and includes the Python API header. Treat generated `*_wrap.cxx` files and Python proxy files as outputs, and make API changes in the headers, implementations, and `.i` files first.

`src/Solver`, `src/Postprocessor`, `src/Library`, and portions of `src/Utility` contain bundled tools, libraries, or third-party sources. 
