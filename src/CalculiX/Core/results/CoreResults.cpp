#include "Core/results/CoreResults.hpp"
#include "CoreResultsIdentity.hpp"
#include <string>
#include <vector>
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/jobs/CalculiXJobRunRecord.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/results/CoreResultsProject.hpp"
#include "Core/results/vtk/CoreResultsVtkLink.hpp"
#include "Core/results/vtk/CoreResultsVtkConverter.hpp"
#include "Framework/Results/VtkResultsExporter.hpp"
#include "CubitInterface.hpp"
#include "Core/topology/CoreTopologyConsoleMessages.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <chrono>
#include <filesystem>
#include <QScopedValueRollback>
#include <QScopeGuard>
#include "ProgressTool.hpp"
#include "AppUtil.hpp"
#include "loadUserOptions.hpp"

namespace
{
  CoreResultsSourceMetadata companion_input_metadata(const CoreResultsFrd& frd, const CoreResultsDat& dat)
  {
    namespace fs = std::filesystem;
    const auto frdPath = fs::u8path(frd.filepath);
    const auto datPath = fs::u8path(dat.filepath);
    auto inputPath = frdPath;
    inputPath.replace_extension(".inp");
    auto expectedDatPath = inputPath;
    expectedDatPath.replace_extension(".dat");
    std::error_code fileError;
    if (datPath.lexically_normal() != expectedDatPath.lexically_normal() ||
        !fs::is_regular_file(inputPath, fileError))
    {
      return {};
    }

    CalculiXJobRunRecord inputMetadata;
    std::string error;
    if (!inputMetadata.read_input_units(inputPath.u8string(), error))
    {
      TOPOLOGY_WARNING("Cannot read result unit metadata from the companion INP: %s\n", error.c_str());
      return {};
    }
    if (inputMetadata.unitSystem.empty() && inputMetadata.step_axis_quantities.empty())
    {
      return {};
    }

    const auto frdTime = fs::last_write_time(frdPath, fileError);
    if (fileError)
    {
      return {};
    }
    const auto datTime = fs::last_write_time(datPath, fileError);
    if (fileError)
    {
      return {};
    }
    const auto predatesResults = [&](const fs::path& path)
    {
      std::error_code timeError;
      const auto inputTime = fs::last_write_time(path, timeError);
      return !timeError && inputTime <= frdTime && inputTime <= datTime;
    };
    std::vector<std::string> inputFiles;
    if (!predatesResults(inputPath) ||
        !CoreResultsVtkLink::match_input_mesh(frd, inputPath.u8string(), inputFiles, error))
    {
      TOPOLOGY_WARNING("Result units remain unknown: the companion INP must predate the outputs and match the FRD mesh. %s\n", error.c_str());
      return {};
    }
    for (const auto& file : inputFiles)
    {
      if (!predatesResults(fs::path(file)))
      {
        TOPOLOGY_WARNING("Result units remain unknown: an input file is newer than the outputs or its timestamp could not be read: %s\n", file.c_str());
        return {};
      }
    }

    // The companion deck supplies units and axes only. A mesh and timestamp
    // match cannot establish an archived launch or project material identity.
    CoreResultsSourceMetadata metadata;
    metadata.connections = std::move(inputMetadata.connections);
    metadata.unitSystem = std::move(inputMetadata.unitSystem);
    metadata.step_axis_quantities = std::move(inputMetadata.step_axis_quantities);
    TOPOLOGY_INFO("Loaded result unit and step-axis metadata from companion INP: %s\n", inputPath.u8string().c_str());
    return metadata;
  }
}

CoreResults::CoreResults()
{}

CoreResults::~CoreResults()
{}

bool CoreResults::init() {
  if (is_initialized) {
    return false; // already initialized
  } else {
    is_initialized = true;
    return true;
  }
}

bool CoreResults::update() {
  return true;
}

bool CoreResults::reset() {
  if (!can_change_readers()) {
    return false;
  }
  clear_vtk_cache();
  retained_identities.clear();
  source_metadata.clear();
  results_data.clear();
  frd_data.clear();
  dat_data.clear();

  init();
  return true;
}

bool CoreResults::check_initialized() {
  return is_initialized;
}

bool CoreResults::create_result(int job_id) {
  if (!can_change_readers()) {
    return false;
  }
  int result_id;
  int result_last;
  CoreResultsFrd frd;
  CoreResultsDat dat;

  if (results_data.size()==0) {
    result_id = 1;
  }
  else {
    result_last = int(results_data.size()) - 1;
    result_id = results_data[result_last][0] + 1;
  }

  this->add_result(result_id, job_id, job_id, job_id);
  // create frd and dat file reader writer object
  frd_data.push_back(frd);
  frd_data[frd_data.size()-1].init(job_id);
  dat_data.push_back(dat);
  dat_data[dat_data.size()-1].init(job_id);

  return true;
}

bool CoreResults::create_frd_dat(int job_id) {
  if (!can_change_readers()) {
    return false;
  }
  CoreResultsFrd frd;
  CoreResultsDat dat;

  // create frd and dat file reader writer object
  frd_data.push_back(frd);
  frd_data[frd_data.size()-1].init(job_id);
  dat_data.push_back(dat);
  dat_data[dat_data.size()-1].init(job_id);

  return true;
}

bool CoreResults::add_result(int result_id, int job_id, int frd_id, int dat_id) {
  if (!can_change_readers()) {
    return false;
  }
  std::vector<int> v = {result_id, job_id, frd_id, dat_id};
  results_data.push_back(v);
  return true;
}

bool CoreResults::delete_result(int job_id) {
  if (!can_change_readers()) {
    return false;
  }
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1) {
    return false;
  } else {
    vtk_results.erase(job_id);
    vtk_exports.erase(job_id);
    vtk_status.erase(job_id);
    frd_data.erase(frd_data.begin() + frd_data_id);
    dat_data.erase(dat_data.begin() + dat_data_id);
    retained_identities.erase(job_id);
    source_metadata.erase(job_id);
    results_data.erase(results_data.begin() + results_data_id);
    return true;
  }
}

bool CoreResults::load_result(int job_id) {
  const CoreTopologyConsole log(CoreTopologyConsole::job_context(job_id));
  if (!can_change_readers()) {
    return false;
  }
  retained_identities.erase(job_id);
  source_metadata.erase(job_id);
  vtk_results.erase(job_id);
  vtk_exports.erase(job_id);
  vtk_status.erase(job_id);
  CalculiXJobsInterface().set_job_conversion(job_id, -1);
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1) {
    return false;
  } else {
    //log = "Loading results for Job ID " + std::to_string(results_data[results_data_id][1]) + " \n";
    //PRINT_INFO("%s", log.c_str());
    std::shared_ptr<const CalculiXJobRunRecord> loadedRun;
    std::string metadataError;
    const bool archived = CalculiXJobsInterface().get_run_record(job_id, loadedRun, metadataError) && loadedRun;
    std::error_code fileError;
    const bool discreteDatOnly = archived && !loadedRun->connections.discrete.empty() &&
      !std::filesystem::exists(std::filesystem::u8path(frd_data[frd_data_id].filepath),fileError) && !fileError;
    if (discreteDatOnly)
    {
      frd_data[frd_data_id].clear();
    }
    else if (!frd_data[frd_data_id].read()) {
      log = "Can't read FRD! \n";
      TOPOLOGY_ERROR("%s", log.c_str());
      return false;
    }
    if (!dat_data[dat_data_id].read()) {
      log = "Can't read DAT! \n";
      TOPOLOGY_ERROR("%s", log.c_str());
      return false;
    }
    if (archived) {
      source_metadata.emplace(job_id, CoreResultsSourceMetadata(*loadedRun));
    }
    else
    {
      auto metadata = companion_input_metadata(frd_data[frd_data_id], dat_data[dat_data_id]);
      if (!metadata.unitSystem.empty() || !metadata.step_axis_quantities.empty())
      {
        source_metadata.emplace(job_id, std::move(metadata));
      }
    }
    return true;
  }
}

SimResults::ConversionStatus CoreResults::get_vtk_status(int job_id) const
{
  const auto found = vtk_status.find(job_id);
  if (found == vtk_status.end()) {
    return SimResults::ConversionStatus::NotConverted;
  }
  return found->second;
}

bool CoreResults::prepare_vtk_plan(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan) {
  const CoreTopologyConsole log(CoreTopologyConsole::job_context(job_id));
  if (vtk_build_active) {
    TOPOLOGY_ERROR("A VTK result build is already active.\n");
    return false;
  }
  QScopedValueRollback<bool> guard(vtk_build_active, true);
  return prepare_vtk_plan_impl(job_id, options, plan);
}

bool CoreResults::prepare_vtk_plan_impl(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan) {
  const int frd_id = get_frd_data_id_from_job_id(job_id);
  const int dat_id = get_dat_data_id_from_job_id(job_id);
  CalculiXJobsInterface jobs;
  const auto job = jobs.get_job_data(job_id);
  if (get_results_data_id_from_job_id(job_id) < 0 || frd_id < 0 || dat_id < 0 || static_cast<std::size_t>(frd_id) >= frd_data.size() || static_cast<std::size_t>(dat_id) >= dat_data.size() || job.size() < 4 || job[3] == "1") {
    TOPOLOGY_ERROR("Cannot plan VTK results for a missing or running job.\n");
    return false;
  }
  vtk_status[job_id] = SimResults::ConversionStatus::Planning;
  CoreResultsVtkLink linker;
  try {
    std::string sourceSystem;
    const auto sourceMetadata = source_metadata.find(job_id);

    if (sourceMetadata != source_metadata.end())
    {
      sourceSystem = sourceMetadata->second.unitSystem;
    }

    if (!linker.prepare(job_id, frd_data[frd_id], dat_data[dat_id], job[1] + ".inp", options, plan, sourceSystem,
        sourceMetadata == source_metadata.end() ? nullptr : &sourceMetadata->second.connections))
    {
      vtk_status[job_id] = SimResults::ConversionStatus::Failed;
      return false;
    }
    plan.source_workspace = "calculix";
    std::map<int, int> solverMaterials;
    std::map<int, int> materialIds;
    for (const auto& row : frd_data[frd_id].elements) {
      if (row.size() >= 4) {
        solverMaterials.emplace(row[0], row[3]);
        materialIds.emplace(row[3], 0);
      }
    }
    std::string metadataError;
    const auto metadata = source_metadata.find(job_id);
    if (metadata != source_metadata.end()) {
      const auto* record = &metadata->second;
      plan.mesh.connections = record->connections;
      plan.source_units.system = record->unitSystem;
      plan.step_axis_quantities = record->step_axis_quantities;
      plan.solver_launch_id = record->launchId;
      plan.source_project = record->materialProjectId;
      if (!record->unitSystem.empty()) {
        SimUnits::QuantityUnit length;
        SimUnits::QuantityUnit time;
        if (!framework_units().system_unit(record->unitSystem, "length", length, metadataError) || !framework_units().system_unit(record->unitSystem, "time", time, metadataError)) {
          TOPOLOGY_ERROR("Cannot read recorded result units: %s\n", metadataError.c_str());
          vtk_status[job_id] = SimResults::ConversionStatus::Failed;
          return false;
        }
        plan.source_units.coordinate_unit = length.unit;
        plan.source_units.time_unit = time.unit;
      }
      for (const auto& material : frd_data[frd_id].materials) {
        if (material.size() < 2) {
          continue;
        }
        const auto header = material[0] + material[1];
        int matchedOrdinal = 0;
        int matchedId = 0;
        int matches = 0;
        for (const auto& mapping : materialIds) {
          const auto prefix = std::to_string(mapping.first);
          if (header.compare(0, prefix.size(), prefix) != 0) {
            continue;
          }
          const auto mapped = record->materialIds.find(header.substr(prefix.size()));
          if (mapped != record->materialIds.end()) {
            matchedOrdinal = mapping.first;
            matchedId = mapped->second;
            ++matches;
          }
        }
        // The legacy FRD reader joins the ordinal and name. Ambiguous prefixes remain unresolved.
        if (matches == 1) {
          materialIds.at(matchedOrdinal) = matchedId;
        }
      }
    }
    for (auto& element : plan.mesh.elements) {
      const auto raw = solverMaterials.find(element.id);
      if (raw == solverMaterials.end() || plan.mesh.connections.discrete.count(element.id)) {
        continue;
      }
      element.solver_material_id = raw->second;
      element.material_id = materialIds.at(raw->second);
    }
  }
  catch (const std::exception& error) {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    TOPOLOGY_ERROR("VTK planning failed: %s\n", error.what());
    return false;
  }
  catch (...) {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    TOPOLOGY_ERROR("Unexpected VTK planning failure.\n");
    return false;
  }
  vtk_status[job_id] = SimResults::ConversionStatus::Planned;
  return true;
}

bool CoreResults::can_change_readers() const
{
  if (vtk_build_active) {
    TOPOLOGY_ERROR("Cannot modify result readers while VTK planning/conversion is active.\n");
    return false;
  }
  return true;
}

void CoreResults::clear_vtk_cache() {
  vtk_results.clear();
  vtk_exports.clear();
  vtk_status.clear();
}

bool CoreResults::is_vtk_build_active() const
{
  return vtk_build_active;
}

bool CoreResults::cancel_vtk_build() {
  if (!vtk_build_active) {
    return false;
  }
  vtk_cancelled.store(true, std::memory_order_relaxed);
  return true;
}

bool CoreResults::build_vtk_result(int job_id, const CoreResultsVtkOptions& options) {
  const CoreTopologyConsole log(CoreTopologyConsole::job_context(job_id));
  if (vtk_build_active) {
    TOPOLOGY_ERROR("A VTK result build is already active.\n");
    return false;
  }
  QScopedValueRollback<bool> guard(vtk_build_active, true);
  const auto conversion_begin = std::chrono::steady_clock::now();
  CalculiXJobsInterface().set_job_conversion(job_id, -1);
  vtk_cancelled.store(false, std::memory_order_relaxed);
  try {
    CoreResultsVtkOptions resolved = options;
    if (resolved.workers == 0) {
      if (ccx_uo.mConverterThreads < 1) {
        vtk_status[job_id] = SimResults::ConversionStatus::Failed;
        TOPOLOGY_ERROR("VTK conversion requires at least one converter thread.\n");
        return false;
      }
      resolved.workers = static_cast<std::size_t>(ccx_uo.mConverterThreads);
    }
    CoreResultsVtkPlan plan;
    const auto planning_begin = std::chrono::steady_clock::now();
    if (!prepare_vtk_plan_impl(job_id, resolved, plan)) {
      return false;
    }
    const double planning_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - planning_begin).count();
    plan.run_identity = std::to_string(job_id) + ":" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const int frd_id = get_frd_data_id_from_job_id(job_id);
    const int dat_id = get_dat_data_id_from_job_id(job_id);
    plan.result_identity = retained_result_identity(frd_data[frd_id],dat_data[dat_id],plan);
    ProgressTool* progress = CubitInterface::app_util().get()->progress_tool();
    if (!progress) {
      vtk_status[job_id] = SimResults::ConversionStatus::Failed;
      TOPOLOGY_ERROR("VTK conversion progress service is unavailable.\n");
      return false;
    }
    progress->start(0, 100, "Building VTK Result Datasets");
    const auto end_progress = qScopeGuard([&]() {
      progress->end();
    });
    auto previous = std::chrono::steady_clock::time_point{};
    const auto poll = [&](double fraction) {
      const auto now = std::chrono::steady_clock::now();
      if (now - previous >= std::chrono::milliseconds(100) || fraction >= 1.0) {
        previous = now;
        progress->percent(fraction);
        progress->check_interrupt();
        if (CubitInterface::app_util().get()->interrupt()) {
          vtk_cancelled.store(true, std::memory_order_relaxed);
        }
      }
      return !vtk_cancelled.load(std::memory_order_relaxed);
    };
    vtk_status[job_id] = SimResults::ConversionStatus::Converting;
    std::shared_ptr<const SimResults::VtkResultsData> converted;
    std::string error;
    CoreResultsVtkConverter converter;
    CoreResultsVtkTimings timings;
    if (!converter.build(frd_data[frd_id], dat_data[dat_id], plan, vtk_cancelled, poll, converted, error, &timings)) {
      if (error.empty() && vtk_cancelled.load(std::memory_order_relaxed)) {
        vtk_status[job_id] = SimResults::ConversionStatus::Cancelled;
        TOPOLOGY_INFO("VTK result build cancelled.\n");
      }
      else {
        vtk_status[job_id] = SimResults::ConversionStatus::Failed;
        if (error.empty()) {
          error = "unknown conversion error";
        }
        TOPOLOGY_ERROR("VTK result build failed: %s\n", error.c_str());
      }
      return false;
    }
    retained_identities[job_id] = converted->result_identity;
    vtk_results[job_id] = std::move(converted);
    vtk_exports.erase(job_id);
    vtk_status[job_id] = SimResults::ConversionStatus::Complete;
    CalculiXJobsInterface().set_job_conversion(job_id, 1);
    const double conversion_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - conversion_begin).count();
    TOPOLOGY_INFO("Duration of VTK conversion (Boost, %zu threads) [ms]: %.3f\n", resolved.workers, conversion_ms);
    TOPOLOGY_INFO("VTK conversion phases [ms]: planning/linking %.3f, geometry %.3f, field mapping %.3f, IP displacement %.3f, tables %.3f, finalize %.3f.\n",
      planning_ms, timings.geometry_ms, timings.fields_ms, timings.ip_displacement_ms, timings.tables_ms, timings.finalize_ms);
    return true;
  }
  catch (const std::exception& error) {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    TOPOLOGY_ERROR("VTK result build failed: %s\n", error.what());
    return false;
  }
  catch (...) {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    TOPOLOGY_ERROR("Unexpected VTK result build failure.\n");
    return false;
  }
}

std::shared_ptr<const SimResults::VtkResultsData> CoreResults::get_vtk_result(int job_id) const
{
  const auto found = vtk_results.find(job_id);
  if (found == vtk_results.end()) {
    return nullptr;
  }
  return found->second;
}

std::string CoreResults::get_vtk_export_path(int job_id) const
{
  const auto found = vtk_exports.find(job_id);
  if (found == vtk_exports.end()) {
    return {};
  }
  return found->second;
}

bool CoreResults::export_vtk_result(int job_id, const std::string& destination) {
  if (vtk_build_active) {
    TOPOLOGY_ERROR("A VTK result operation is already active.\n");
    return false;
  }
  const auto result = get_vtk_result(job_id);
  if (!result || get_vtk_status(job_id) != SimResults::ConversionStatus::Complete) {
    TOPOLOGY_ERROR("Build a complete VTK result before exporting job %d.\n", job_id);
    return false;
  }
  QScopedValueRollback<bool> guard(vtk_build_active, true);
  const auto export_begin = std::chrono::steady_clock::now();
  vtk_cancelled.store(false, std::memory_order_relaxed);
  try {
    std::string target = destination;
    if (ccx_uo.mConverterThreads < 1) {
      TOPOLOGY_ERROR("VTK export requires at least one converter thread.\n");
      return false;
    }
    if (target.empty()) {
      CalculiXJobsInterface jobs;
      const auto job = jobs.get_job_data(job_id);
      if (job.size() < 2) {
        TOPOLOGY_ERROR("Cannot find the VTK export path for job %d.\n", job_id);
        return false;
      }
      const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
      target = (std::filesystem::u8path(job[1] + ".vtk") / ("run_" + std::to_string(token))).u8string();
    }
    std::string index;
    std::string error;
    std::size_t workers_used = 0;
    if (!SimResults::export_vtk_result(*result, target, index, error,
      static_cast<std::size_t>(ccx_uo.mConverterThreads), [&](double) {
        return !vtk_cancelled.load(std::memory_order_relaxed);
      }, &workers_used)) {
      TOPOLOGY_ERROR("VTK export for job %d failed: %s\n", job_id, error.c_str());
      return false;
    }
    vtk_exports[job_id] = index;
    const double export_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - export_begin).count();
    TOPOLOGY_INFO("Duration of VTK export (%zu threads) [ms]: %.3f\n", workers_used, export_ms);
    TOPOLOGY_INFO("VTK result exported to %s\n", index.c_str());
    return true;
  }
  catch (const std::exception& failure) {
    TOPOLOGY_ERROR("VTK export for job %d failed: %s\n", job_id, failure.what());
    return false;
  }
}

int CoreResults::convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids) {
  CoreResultsVtkOptions settings;
  const bool frd_only = !options.empty() && options[0] == 1;
  const bool dat_only = options.size() > 1 && options[1] == 1;
  settings.include_frd = frd_only || !dat_only;
  settings.include_dat = dat_only || !frd_only;
  settings.block_ids = std::move(block_ids);
  settings.nodeset_ids = std::move(nodeset_ids);
  settings.sideset_ids = std::move(sideset_ids);
  if (!build_vtk_result(job_id, settings)) {
    return -1;
  }
  return 1;
}

bool CoreResults::project_topology(int topology_id,int iteration_id, double threshold, bool preview) {
  CoreResultsProject projector;
  return projector.project_topology(topology_id, iteration_id, threshold, preview);
}

bool CoreResults::project_result(int job_id, int step, int totalincrement,double scale) {
  const auto metadata = source_metadata.find(job_id);
  if (metadata == source_metadata.end() || metadata->second.unitSystem.empty())
  {
    TOPOLOGY_ERROR("Cannot project displacements without recorded result units.\n");
    return false;
  }
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);
  CoreResultsProject projector;

  if (results_data_id == -1 || frd_data_id < 0 || static_cast<std::size_t>(frd_data_id) >= frd_data.size()) {
    return false;
  } else {
    if (step!=-1) {
      log = "Project results for Job ID " + std::to_string(results_data[results_data_id][1]) + " for Step " + std::to_string(step) + " \n";
    } else {
      log = "Project results for Job ID " + std::to_string(results_data[results_data_id][1]) + " for total increment " + std::to_string(totalincrement) + " \n";
    }
    TOPOLOGY_INFO("%s", log.c_str());
    if (!projector.init(&frd_data[frd_data_id], metadata->second.connections) ||
        !projector.project(step, totalincrement, scale, metadata->second.unitSystem))
    {
      return false;
    }

    return true;
  }
}

int CoreResults::get_results_data_id_from_job_id(int job_id) {
  int return_int = -1;
  for (size_t i = 0; i < results_data.size(); i++) {
    if (results_data[i][1]==job_id) {
      return_int = int(i);
    }
  }
  return return_int;
}

int CoreResults::get_frd_data_id_from_job_id(int job_id) {
  int return_int = -1;

  for (size_t i = 0; i < frd_data.size(); i++) {
    if (job_id == frd_data[i].job_id) {
      return_int = int(i);
    }
  }

  return return_int;
}

int CoreResults::get_dat_data_id_from_job_id(int job_id) {
  int return_int = -1;

  for (size_t i = 0; i < dat_data.size(); i++) {
    if (job_id == dat_data[i].job_id) {
      return_int = int(i);
    }
  }

  return return_int;
}

std::string CoreResults::print_data() {
  std::string str_return;
  str_return = "\n CoreResults results_data: \n";
  str_return.append("result_id, job_id, frd_id, dat_id \n");

  for (size_t i = 0; i < results_data.size(); i++) {
    str_return.append(std::to_string(results_data[i][0]) + " " + std::to_string(results_data[i][1]) + " " + std::to_string(results_data[i][2]) + " " + std::to_string(results_data[i][3]) +  " \n");
  }

  return str_return;
}

std::string CoreResults::retained_identity(int job_id, std::string& error) {
  error.clear();
  if (!can_change_readers()) {
    error = "CalculiX reader data is busy.";
    return {};
  }
  CoreResultsVtkPlan plan;
  if (!prepare_vtk_plan_impl(job_id,CoreResultsVtkOptions{},plan)) {
    error = "The saved reader/mesh data cannot reproduce this result.";
    return {};
  }
  const auto frd = get_frd_data_id_from_job_id(job_id), dat = get_dat_data_id_from_job_id(job_id);
  const auto identity = retained_result_identity(frd_data[frd],dat_data[dat],plan);
  const auto saved = retained_identities.find(job_id);
  if (saved != retained_identities.end() && saved->second != identity) {
    error = "Saved result content or mesh-link metadata no longer matches its identity.";
    return {};
  }
  return identity;
}
