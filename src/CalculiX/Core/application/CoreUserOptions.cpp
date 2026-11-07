#include "CoreUserOptions.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "loadUserOptions.hpp"
#include "CubitMessage.hpp"
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <limits>
#include <type_traits>
#include <utility>

namespace
{
UserOptionsInterface* optionsStore = nullptr;
UserOptions configuredOptions;

// Commit prepared snapshots without allocations after persistence succeeds.
// UserOptions has a user-declared destructor, so swapping the entire object
// would copy it; swap each member using its non-throwing operation instead.
void swapOptions(UserOptions& first, UserOptions& second) noexcept
{
  static_assert(std::is_nothrow_swappable<DrawingOptions>::value, "Drawing options must support a non-throwing commit.");
  using std::swap;
  swap(first.mDrawing, second.mDrawing);
  first.mPathSolver.swap(second.mPathSolver);
  first.mPathSolverName.swap(second.mPathSolverName);
  swap(first.mSolverThreads, second.mSolverThreads);
  first.mSolverThreadsName.swap(second.mSolverThreadsName);
  swap(first.mConverterThreads, second.mConverterThreads);
  first.mConverterThreadsName.swap(second.mConverterThreadsName);
  first.mPathCGX.swap(second.mPathCGX);
  first.mPathCGXName.swap(second.mPathCGXName);
  first.mPathParaView.swap(second.mPathParaView);
  first.mPathParaViewName.swap(second.mPathParaViewName);
  first.mPathIcons.swap(second.mPathIcons);
  first.mPathIconsName.swap(second.mPathIconsName);
  first.mPathPythonInterface.swap(second.mPathPythonInterface);
  first.mPathPythonInterfaceName.swap(second.mPathPythonInterfaceName);
  swap(first.mSaveLoadedResults, second.mSaveLoadedResults);
  first.mSaveLoadedResultsName.swap(second.mSaveLoadedResultsName);
  first.mPathMaterialLibrary.swap(second.mPathMaterialLibrary);
  first.mPathMaterialLibraryName.swap(second.mPathMaterialLibraryName);
}

bool samePath(const QString& first, const QString& second)
{
  const QString firstPath = QDir::cleanPath(QDir::fromNativeSeparators(first));
  const QString secondPath = QDir::cleanPath(QDir::fromNativeSeparators(second));
#ifdef _WIN32
  return firstPath.compare(secondPath, Qt::CaseInsensitive) == 0;
#else
  return firstPath == secondPath;
#endif
}

bool positiveInteger(const UserOptionValue& value)
{
  return std::get<int>(value) > 0;
}

bool positiveReal(const UserOptionValue& value)
{
  return std::isfinite(std::get<double>(value)) && std::get<double>(value) > 0;
}

bool validColor(const UserOptionValue& value)
{
  const std::array<std::string, 9> colors =
    {"red", "yellow", "green", "blue", "cyan", "magenta", "white", "black", "orange"};
  return std::find(colors.begin(), colors.end(), std::get<std::string>(value)) != colors.end();
}

bool validPath(const UserOptionValue& value)
{
  const auto& path = std::get<std::string>(value);
  if (path.empty() || path.find('\0') != std::string::npos || path.find_first_of("\"\r\n") != std::string::npos)
  {
    return false;
  }
  const QString normalized = QDir::cleanPath(QDir::fromNativeSeparators(QString::fromUtf8(path.c_str())));
  return QFileInfo(normalized).isAbsolute() ||
    (normalized != ".." && !normalized.startsWith("../"));
}

// The authoritative defaults for every CalculiX user option live here.
// Relative resource paths are resolved against the host component directory;
// existence is checked when the resource is used, not when options are loaded.
std::vector<UserOption> definitions()
{
#ifdef _WIN32
  const char* solverDefault = "Solver/ccx_2.23.exe";
  const char* cgxDefault = "Postprocessor/CGX/cgx_STATIC.exe";
  const char* paraviewDefault = "Postprocessor/ParaView/bin/paraview.exe";
#else
  const char* solverDefault = "Solver/ccx_2.22_MT";
  const char* cgxDefault = "Postprocessor/CGX/cgx_2.22";
  const char* paraviewDefault = "Postprocessor/ParaView/bin/paraview";
#endif
  return {
    {"paths/solver", UserOptionKind::ComponentPath, std::string(solverDefault), validPath},
    {"paths/cgx", UserOptionKind::ComponentPath, std::string(cgxDefault), validPath},
    {"paths/paraview", UserOptionKind::ComponentPath, std::string(paraviewDefault), validPath},
    {"paths/icons", UserOptionKind::ComponentPath, std::string("Icons/"), validPath},
    {"paths/python_interface", UserOptionKind::ComponentPath, std::string("."), validPath},
    {"paths/material_library", UserOptionKind::ComponentPath, std::string("libmaterial.hdf5"), validPath},
    {"execution/solver_threads", UserOptionKind::Integer, 8, positiveInteger},
    {"execution/converter_threads", UserOptionKind::Integer, 8, positiveInteger},
    {"results/save_loaded_results", UserOptionKind::Boolean, true},
    {"drawing/size_factor", UserOptionKind::Real, 1.0, positiveReal},
    {"drawing/overlay_visible", UserOptionKind::Boolean, true},
    {"drawing/boundary_conditions", UserOptionKind::Boolean, false},
    {"drawing/loads", UserOptionKind::Boolean, false},
    {"drawing/constraints", UserOptionKind::Boolean, false},
    {"drawing/orientations", UserOptionKind::Boolean, false},
    {"drawing/hover_color", UserOptionKind::Text, std::string("red"), validColor},
    {"drawing/selection_color", UserOptionKind::Text, std::string("yellow"), validColor}
  };
}

QString resolvedPath(const std::string& value)
{
  const QString path = QDir::fromNativeSeparators(QString::fromUtf8(value.c_str()));
  return QDir::cleanPath(QFileInfo(path).isAbsolute() ? path :
    QDir(QString::fromUtf8(optionsStore->component_directory().c_str())).absoluteFilePath(path));
}

UserOptions runtimeOptions(const UserOptionValues& values)
{
  UserOptions result;
  result.mPathSolver = resolvedPath(std::get<std::string>(values.at("paths/solver")));
  result.mPathCGX = resolvedPath(std::get<std::string>(values.at("paths/cgx")));
  result.mPathParaView = resolvedPath(std::get<std::string>(values.at("paths/paraview")));
  result.mPathIcons = resolvedPath(std::get<std::string>(values.at("paths/icons")));
  result.mPathPythonInterface = resolvedPath(std::get<std::string>(values.at("paths/python_interface")));
  result.mPathMaterialLibrary = resolvedPath(std::get<std::string>(values.at("paths/material_library")));
  result.mSolverThreads = std::get<int>(values.at("execution/solver_threads"));
  result.mConverterThreads = std::get<int>(values.at("execution/converter_threads"));
  result.mSaveLoadedResults = std::get<bool>(values.at("results/save_loaded_results"));
  result.mDrawing.sizeFactor = std::get<double>(values.at("drawing/size_factor"));
  result.mDrawing.overlayVisible = std::get<bool>(values.at("drawing/overlay_visible"));
  result.mDrawing.boundaryConditions = std::get<bool>(values.at("drawing/boundary_conditions"));
  result.mDrawing.loads = std::get<bool>(values.at("drawing/loads"));
  result.mDrawing.constraints = std::get<bool>(values.at("drawing/constraints"));
  result.mDrawing.orientations = std::get<bool>(values.at("drawing/orientations"));
  result.mDrawing.hoverColor = std::get<std::string>(values.at("drawing/hover_color"));
  result.mDrawing.selectionColor = std::get<std::string>(values.at("drawing/selection_color"));
  result.mPathSolverName = "Path to CalculiX Solver";
  result.mSolverThreadsName = "Solver - Threads";
  result.mConverterThreadsName = "Converter - Threads";
  result.mPathCGXName = "Path to CGX";
  result.mPathParaViewName = "Path to ParaView";
  result.mPathIconsName = "Path to Icons";
  result.mPathPythonInterfaceName = "Path to Python Interface";
  result.mSaveLoadedResultsName = "Save loaded FRD and DAT data";
  result.mPathMaterialLibraryName = "Path to MaterialLibrary";
  return result;
}


}

bool CoreUserOptions::initialize(UserOptionsInterface& store, std::string& error)
{
  optionsStore = &store;
  UserOptionValues values;
  if (!store.load("calculix", definitions(), values, error))
  {
    optionsStore = nullptr;
    return false;
  }
  auto configured = runtimeOptions(values);
  auto active = configured;
  swapOptions(configuredOptions, configured);
  swapOptions(ccx_uo, active);
  return true;
}

void CoreUserOptions::detach() noexcept
{
  optionsStore = nullptr;
}

UserOptions CoreUserOptions::current()
{
  return optionsStore == nullptr ? ccx_uo : configuredOptions;
}

std::vector<std::string> CoreUserOptions::restart_required()
{
  std::vector<std::string> labels;
  if (optionsStore == nullptr)
  {
    return labels;
  }
  if (!samePath(configuredOptions.mPathIcons, ccx_uo.mPathIcons))
  {
    labels.push_back(configuredOptions.mPathIconsName.toStdString());
  }
  if (!samePath(configuredOptions.mPathPythonInterface, ccx_uo.mPathPythonInterface))
  {
    labels.push_back(configuredOptions.mPathPythonInterfaceName.toStdString());
  }
  if (!samePath(configuredOptions.mPathMaterialLibrary, ccx_uo.mPathMaterialLibrary))
  {
    labels.push_back(configuredOptions.mPathMaterialLibraryName.toStdString());
  }
  return labels;
}

std::string CoreUserOptions::component_directory()
{
  return optionsStore == nullptr ? std::string() : optionsStore->component_directory();
}

bool CoreUserOptions::apply(const UserOptionValues& changes, std::string& error)
{
  error.clear();
  if (optionsStore == nullptr)
  {
    error = "CalculiX user options are not initialized.";
    return false;
  }
  try
  {
    const auto schema = definitions();
    UserOptionValues values;
    if (!optionsStore->load("calculix", schema, values, error) ||
      !merge_user_option_changes(schema, changes, values, error))
    {
      return false;
    }
    for (const auto& item : schema)
    {
      if (item.kind == UserOptionKind::ComponentPath)
      {
        const auto path = QDir::fromNativeSeparators(QString::fromUtf8(std::get<std::string>(values.at(item.key)).c_str()));
        const auto relative = QDir(QString::fromUtf8(optionsStore->component_directory().c_str())).relativeFilePath(QDir::cleanPath(path));
        if (QFileInfo(path).isAbsolute() && relative != ".." && !relative.startsWith("../") && !QFileInfo(relative).isAbsolute())
        {
          values[item.key] = relative.toUtf8().toStdString();
        }
      }
    }
    // Prepare both snapshots before saving. Keep resources already loaded by
    // this session on their active paths until Cubit is restarted.
    auto configured = runtimeOptions(values);
    auto active = configured;
    active.mPathIcons = ccx_uo.mPathIcons;
    active.mPathPythonInterface = ccx_uo.mPathPythonInterface;
    active.mPathMaterialLibrary = ccx_uo.mPathMaterialLibrary;
    if (!optionsStore->save("calculix", schema, values, error))
    {
      return false;
    }
    swapOptions(configuredOptions, configured);
    swapOptions(ccx_uo, active);
    // Drawing size/visibility remain defaults for new/reset models; colors
    // update the current view, matching the existing options-panel behavior.
    try
    {
      std::string drawingError;
      if (!CalculiXDrawInterface().apply_drawing_options(drawingError))
      {
        PRINT_WARNING("Options were saved, but drawing colors could not be refreshed: %s\n", drawingError.c_str());
      }
    }
    catch (const std::exception& exception)
    {
      PRINT_WARNING("Options were saved, but drawing colors could not be refreshed: %s\n", exception.what());
    }
    catch (...)
    {
      PRINT_WARNING("Options were saved, but drawing colors could not be refreshed.\n");
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

std::vector<UserOption> CoreUserOptions::user_option_definitions()
{
  return definitions();
}

bool CoreUserOptions::values(UserOptionValues& snapshot, std::string& error)
{
  if (optionsStore == nullptr)
  {
    error = "CalculiX user options are unavailable.";
    return false;
  }
  return optionsStore->load("calculix", definitions(), snapshot, error);
}
