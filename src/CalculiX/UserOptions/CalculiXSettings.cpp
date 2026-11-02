#include "CalculiXSettings.hpp"
#include "CalculiXConfigFile.hpp"
#include "loadUserOptions.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>

namespace
{
bool validDrawingColor(const std::string& color)
{
  const std::array<std::string, 9> colors = {"red", "yellow", "green", "blue", "cyan", "magenta", "white", "black", "orange"};
  return std::find(colors.begin(), colors.end(), color) != colors.end();
}

void readDrawingOptions(CalculiXConfigFile& config, DrawingOptions& options)
{
  options = DrawingOptions{};
  config.read_double_entry("DrawingSizeFactor", options.sizeFactor);
  std::ifstream input(config.filepath);
  for (std::string line; std::getline(input, line);)
  {
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }
    const auto separator = line.find('=');
    if (separator == std::string::npos)
    {
      continue;
    }
    const std::string key = line.substr(0, separator);
    const std::string value = line.substr(separator + 1);
    if (key == "DrawingHoverColor" && validDrawingColor(value))
    {
      options.hoverColor = value;
    }
    else if (key == "DrawingSelectionColor" && validDrawingColor(value))
    {
      options.selectionColor = value;
    }
    else if (value == "true" || value == "false")
    {
      const bool enabled = value == "true";
      if (key == "DrawingOverlayVisible")
      {
        options.overlayVisible = enabled;
      }
      else if (key == "DrawingBoundaryConditions")
      {
        options.boundaryConditions = enabled;
      }
      else if (key == "DrawingLoads")
      {
        options.loads = enabled;
      }
      else if (key == "DrawingConstraints")
      {
        options.constraints = enabled;
      }
      else if (key == "DrawingOrientations")
      {
        options.orientations = enabled;
      }
    }
  }
}

void writeDrawingOptions(CalculiXConfigFile& config, const DrawingOptions& options)
{
  config.write_double_entry("DrawingSizeFactor", std::isfinite(options.sizeFactor) && options.sizeFactor > 0 ? options.sizeFactor : 1.0);
  config.write_bool_entry("DrawingOverlayVisible", options.overlayVisible);
  config.write_bool_entry("DrawingBoundaryConditions", options.boundaryConditions);
  config.write_bool_entry("DrawingLoads", options.loads);
  config.write_bool_entry("DrawingConstraints", options.constraints);
  config.write_bool_entry("DrawingOrientations", options.orientations);
  config.write_entry("DrawingHoverColor", QString::fromStdString(validDrawingColor(options.hoverColor) ? options.hoverColor : "red"));
  config.write_entry("DrawingSelectionColor", QString::fromStdString(validDrawingColor(options.selectionColor) ? options.selectionColor : "yellow"));
}
}

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
  readDrawingOptions(config, ccx_uo.mDrawing);
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
  writeDrawingOptions(config, ccx_uo.mDrawing);
}
