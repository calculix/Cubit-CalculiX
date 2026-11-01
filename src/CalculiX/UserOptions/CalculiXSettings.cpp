#include "CalculiXSettings.hpp"
#include "CalculiXConfigFile.hpp"
#include "loadUserOptions.hpp"

void CalculiXSettings::restore()
{

  CalculiXConfigFile config;
  
  ccx_uo.mPathSolver = config.standard_entry("PathSolver");
  config.read_entry("PathSolver", ccx_uo.mPathSolver);
  ccx_uo.mPathSolverName = "Path to CalculiX Solver";
  ccx_uo.mSolverThreads = config.standard_num_entry("SolverThreads");
  config.read_num_entry("SolverThreads", ccx_uo.mSolverThreads);
  ccx_uo.mSolverThreadsName = "Solver - Threads";
  ccx_uo.mConverterThreads = config.standard_num_entry("ConverterThreads");
  config.read_num_entry("ConverterThreads", ccx_uo.mConverterThreads);
  ccx_uo.mConverterThreadsName = "Converter - Threads";
  ccx_uo.mPathCGX = config.standard_entry("PathCGX");
  config.read_entry("PathCGX", ccx_uo.mPathCGX);
  ccx_uo.mPathCGXName = "Path to CGX";
  ccx_uo.mPathParaView = config.standard_entry("PathParaView");
  config.read_entry("PathParaView", ccx_uo.mPathParaView);
  ccx_uo.mPathParaViewName = "Path to ParaView";
  ccx_uo.mPathIcons = config.standard_entry("PathIcons");
  config.read_entry("PathIcons", ccx_uo.mPathIcons);
  ccx_uo.mPathIconsName = "Path to Icons";
  ccx_uo.mPathPythonInterface = config.standard_entry("PathPythonInterface");
  config.read_entry("PathPythonInterface", ccx_uo.mPathPythonInterface);
  ccx_uo.mPathPythonInterfaceName = "Path to Python Interface";
  ccx_uo.mSaveLoadedResults = config.standard_bool_entry("SaveLoadedResults");
  config.read_bool_entry("SaveLoadedResults", ccx_uo.mSaveLoadedResults);
  ccx_uo.mSaveLoadedResultsName = "Save loaded FRD and DAT data";
  ccx_uo.mPathMaterialLibrary = config.standard_entry("PathMaterialLibrary");
  config.read_entry("PathMaterialLibrary", ccx_uo.mPathMaterialLibrary);
  ccx_uo.mPathMaterialLibraryName = "Path to MaterialLibrary";
}

void CalculiXSettings::save()
{ 
  CalculiXConfigFile config;  
  config.clear();
  config.write_entry("PathSolver", ccx_uo.mPathSolver);
  config.write_num_entry("SolverThreads", ccx_uo.mSolverThreads);
  config.write_num_entry("ConverterThreads", ccx_uo.mConverterThreads);
  config.write_entry("PathCGX", ccx_uo.mPathCGX);
  config.write_entry("PathParaView", ccx_uo.mPathParaView);
  config.write_entry("PathIcons", ccx_uo.mPathIcons);
  config.write_entry("PathPythonInterface", ccx_uo.mPathPythonInterface);
  config.write_bool_entry("SaveLoadedResults", ccx_uo.mSaveLoadedResults);
  config.write_entry("PathMaterialLibrary", ccx_uo.mPathMaterialLibrary);
}
