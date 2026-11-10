#include "Core/results/CoreResults.hpp"
#include "CoreResultsIdentity.hpp"
#include <string>
#include <vector>
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/results/CoreResultsProject.hpp"
#include "Core/results/vtk/CoreResultsVtkLink.hpp"
#include "Core/results/vtk/CoreResultsVtkConverter.hpp"
#include "Framework/Results/VtkResultsExporter.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

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

CoreResults::CoreResults()
{}

CoreResults::~CoreResults()
{}

bool CoreResults::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreResults::update()
{ 
  return true;
}

bool CoreResults::reset()
{
  if (!can_change_readers())
  {
    return false;
  }
  clear_vtk_cache();
  retained_identities.clear();
  results_data.clear();
  frd_data.clear();
  dat_data.clear();

  init();
  return true;
}

bool CoreResults::check_initialized()
{
  return is_initialized;
}

bool CoreResults::create_result(int job_id)
{
  if (!can_change_readers())
  {
    return false;
  }
  int result_id;
  int result_last;
  CoreResultsFrd frd;
  CoreResultsDat dat;
  
  if (results_data.size()==0)
  {
    result_id = 1;
  }
  else
  {
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

bool CoreResults::create_frd_dat(int job_id)
{
  if (!can_change_readers())
  {
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

bool CoreResults::add_result(int result_id, int job_id, int frd_id, int dat_id)
{
  if (!can_change_readers())
  {
    return false;
  }
  std::vector<int> v = {result_id, job_id, frd_id, dat_id};
  results_data.push_back(v);
  return true;
}

bool CoreResults::delete_result(int job_id)
{
  if (!can_change_readers())
  {
    return false;
  }
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return false;
  } else {
    vtk_results.erase(job_id);
    vtk_exports.erase(job_id);
    vtk_status.erase(job_id);
    frd_data.erase(frd_data.begin() + frd_data_id);
    dat_data.erase(dat_data.begin() + dat_data_id);
    retained_identities.erase(job_id);
    results_data.erase(results_data.begin() + results_data_id);
    return true;
  }
}

bool CoreResults::load_result(int job_id)
{
  if (!can_change_readers())
  {
    return false;
  }
  retained_identities.erase(job_id);
  vtk_results.erase(job_id);
  vtk_exports.erase(job_id);
  vtk_status.erase(job_id);
  CalculiXJobsInterface().set_job_conversion(job_id, -1);
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return false;
  } else {
    //log = "Loading results for Job ID " + std::to_string(results_data[results_data_id][1]) + " \n";
    //PRINT_INFO("%s", log.c_str());
    if (!frd_data[frd_data_id].read())
    {
      log = "Can't read FRD! \n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    if (!dat_data[dat_data_id].read())
    {
      log = "Can't read DAT! \n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    return true;
  }
}

SimResults::ConversionStatus CoreResults::get_vtk_status(int job_id) const
{
  const auto found = vtk_status.find(job_id);
  return found == vtk_status.end() ? SimResults::ConversionStatus::NotConverted : found->second;
}

bool CoreResults::prepare_vtk_plan(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan)
{
  if (vtk_build_active)
  {
    PRINT_ERROR("A VTK result build is already active.\n");
    return false;
  }
  QScopedValueRollback<bool> guard(vtk_build_active, true);
  return prepare_vtk_plan_impl(job_id, options, plan);
}

bool CoreResults::prepare_vtk_plan_impl(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan)
{
  const int frd_id = get_frd_data_id_from_job_id(job_id);
  const int dat_id = get_dat_data_id_from_job_id(job_id);
  CalculiXJobsInterface jobs;
  const auto job = jobs.get_job_data(job_id);
  if (get_results_data_id_from_job_id(job_id) < 0 || frd_id < 0 || dat_id < 0 || static_cast<std::size_t>(frd_id) >= frd_data.size() || static_cast<std::size_t>(dat_id) >= dat_data.size() || job.size() < 4 || job[3] == "1")
  {
    PRINT_ERROR("Cannot plan VTK results for a missing or running job.\n");
    return false;
  }
  vtk_status[job_id] = SimResults::ConversionStatus::Planning;
  CoreResultsVtkLink linker;
  try
  {
    if (!linker.prepare(job_id, frd_data[frd_id], dat_data[dat_id], job[1] + ".inp", options, plan))
    {
      vtk_status[job_id] = SimResults::ConversionStatus::Failed;
      return false;
    }
  }
  catch (const std::exception& error)
  {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    PRINT_ERROR("VTK planning failed: %s\n", error.what());
    return false;
  }
  catch (...)
  {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    PRINT_ERROR("Unexpected VTK planning failure.\n");
    return false;
  }
  vtk_status[job_id] = SimResults::ConversionStatus::Planned;
  return true;
}

bool CoreResults::can_change_readers() const
{
  if (vtk_build_active)
  {
    PRINT_ERROR("Cannot modify result readers while VTK planning/conversion is active.\n");
    return false;
  }
  return true;
}

void CoreResults::clear_vtk_cache()
{
  vtk_results.clear();
  vtk_exports.clear();
  vtk_status.clear();
}

bool CoreResults::is_vtk_build_active() const
{
  return vtk_build_active;
}

bool CoreResults::cancel_vtk_build()
{
  if (!vtk_build_active)
  {
    return false;
  }
  vtk_cancelled.store(true, std::memory_order_relaxed);
  return true;
}

bool CoreResults::build_vtk_result(int job_id, const CoreResultsVtkOptions& options)
{
  if (vtk_build_active)
  {
    PRINT_ERROR("A VTK result build is already active.\n");
    return false;
  }
  QScopedValueRollback<bool> guard(vtk_build_active, true);
  const auto conversion_begin = std::chrono::steady_clock::now();
  CalculiXJobsInterface().set_job_conversion(job_id, -1);
  vtk_cancelled.store(false, std::memory_order_relaxed);
  try
  {
    CoreResultsVtkOptions resolved = options;
    if (resolved.workers == 0)
    {
      if (ccx_uo.mConverterThreads < 1)
      {
        vtk_status[job_id] = SimResults::ConversionStatus::Failed;
        PRINT_ERROR("VTK conversion requires at least one converter thread.\n");
        return false;
      }
      resolved.workers = static_cast<std::size_t>(ccx_uo.mConverterThreads);
    }
    CoreResultsVtkPlan plan;
    const auto planning_begin = std::chrono::steady_clock::now();
    if (!prepare_vtk_plan_impl(job_id, resolved, plan))
    {
      return false;
    }
    const double planning_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - planning_begin).count();
    plan.run_identity = std::to_string(job_id) + ":" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const int frd_id = get_frd_data_id_from_job_id(job_id);
    const int dat_id = get_dat_data_id_from_job_id(job_id);
    plan.result_identity = retained_result_identity(frd_data[frd_id],dat_data[dat_id],plan);
    ProgressTool* progress = CubitInterface::app_util().get()->progress_tool();
    if (!progress)
    {
      vtk_status[job_id] = SimResults::ConversionStatus::Failed;
      PRINT_ERROR("VTK conversion progress service is unavailable.\n");
      return false;
    }
    progress->start(0, 100, "Building VTK Result Datasets");
    const auto end_progress = qScopeGuard([&]()
    {
      progress->end();
    });
    auto previous = std::chrono::steady_clock::time_point{};
    const auto poll = [&](double fraction)
    {
      const auto now = std::chrono::steady_clock::now();
      if (now - previous >= std::chrono::milliseconds(100) || fraction >= 1.0)
      {
        previous = now;
        progress->percent(fraction);
        progress->check_interrupt();
        if (CubitInterface::app_util().get()->interrupt())
        {
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
    if (!converter.build(frd_data[frd_id], dat_data[dat_id], plan, vtk_cancelled, poll, converted, error, &timings))
    {
      if (error.empty() && vtk_cancelled.load(std::memory_order_relaxed))
      {
        vtk_status[job_id] = SimResults::ConversionStatus::Cancelled;
        PRINT_INFO("VTK result build cancelled.\n");
      }
      else
      {
        vtk_status[job_id] = SimResults::ConversionStatus::Failed;
        PRINT_ERROR("VTK result build failed: %s\n", error.empty() ? "unknown conversion error" : error.c_str());
      }
      return false;
    }
    retained_identities[job_id] = converted->result_identity;
    vtk_results[job_id] = std::move(converted);
    vtk_exports.erase(job_id);
    vtk_status[job_id] = SimResults::ConversionStatus::Complete;
    CalculiXJobsInterface().set_job_conversion(job_id, 1);
    const double conversion_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - conversion_begin).count();
    PRINT_INFO("Duration of VTK conversion (Boost, %zu threads) [ms]: %.3f\n", resolved.workers, conversion_ms);
    PRINT_INFO("VTK conversion phases [ms]: planning/linking %.3f, geometry %.3f, field mapping %.3f, IP displacement %.3f, tables %.3f, finalize %.3f.\n",
      planning_ms, timings.geometry_ms, timings.fields_ms, timings.ip_displacement_ms, timings.tables_ms, timings.finalize_ms);
    return true;
  }
  catch (const std::exception& error)
  {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    PRINT_ERROR("VTK result build failed: %s\n", error.what());
    return false;
  }
  catch (...)
  {
    vtk_status[job_id] = SimResults::ConversionStatus::Failed;
    PRINT_ERROR("Unexpected VTK result build failure.\n");
    return false;
  }
}

std::shared_ptr<const SimResults::VtkResultsData> CoreResults::get_vtk_result(int job_id) const
{
  const auto found = vtk_results.find(job_id);
  return found == vtk_results.end() ? nullptr : found->second;
}

std::string CoreResults::get_vtk_export_path(int job_id) const
{
  const auto found = vtk_exports.find(job_id);
  return found == vtk_exports.end() ? std::string{} : found->second;
}

bool CoreResults::export_vtk_result(int job_id, const std::string& destination)
{
  if (vtk_build_active)
  {
    PRINT_ERROR("A VTK result operation is already active.\n");
    return false;
  }
  const auto result = get_vtk_result(job_id);
  if (!result || get_vtk_status(job_id) != SimResults::ConversionStatus::Complete)
  {
    PRINT_ERROR("Build a complete VTK result before exporting job %d.\n", job_id);
    return false;
  }
  QScopedValueRollback<bool> guard(vtk_build_active, true);
  const auto export_begin = std::chrono::steady_clock::now();
  vtk_cancelled.store(false, std::memory_order_relaxed);
  try
  {
    std::string target = destination;
    if (ccx_uo.mConverterThreads < 1)
    {
      PRINT_ERROR("VTK export requires at least one converter thread.\n");
      return false;
    }
    if (target.empty())
    {
      CalculiXJobsInterface jobs;
      const auto job = jobs.get_job_data(job_id);
      if (job.size() < 2)
      {
        PRINT_ERROR("Cannot find the VTK export path for job %d.\n", job_id);
        return false;
      }
      const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
      target = (std::filesystem::u8path(job[1] + ".vtk") / ("run_" + std::to_string(token))).u8string();
    }
    std::string index;
    std::string error;
    std::size_t workers_used = 0;
    if (!SimResults::export_vtk_result(*result, target, index, error,
      static_cast<std::size_t>(ccx_uo.mConverterThreads), [&](double)
      {
        return !vtk_cancelled.load(std::memory_order_relaxed);
      }, &workers_used))
    {
      PRINT_ERROR("VTK export for job %d failed: %s\n", job_id, error.c_str());
      return false;
    }
    vtk_exports[job_id] = index;
    const double export_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - export_begin).count();
    PRINT_INFO("Duration of VTK export (%zu threads) [ms]: %.3f\n", workers_used, export_ms);
    PRINT_INFO("VTK result exported to %s\n", index.c_str());
    return true;
  }
  catch (const std::exception& failure)
  {
    PRINT_ERROR("VTK export for job %d failed: %s\n", job_id, failure.what());
    return false;
  }
}

int CoreResults::convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids)
{
  CoreResultsVtkOptions settings;
  const bool frd_only = !options.empty() && options[0] == 1;
  const bool dat_only = options.size() > 1 && options[1] == 1;
  settings.include_frd = frd_only || !dat_only;
  settings.include_dat = dat_only || !frd_only;
  settings.block_ids = std::move(block_ids);
  settings.nodeset_ids = std::move(nodeset_ids);
  settings.sideset_ids = std::move(sideset_ids);
  if (!build_vtk_result(job_id, settings))
  {
    return -1;
  }
  return 1;
}

bool CoreResults::project_topology(int topology_id,int iteration_id, double threshold, bool preview)
{
  CoreResultsProject projector;
  return projector.project_topology(topology_id, iteration_id, threshold, preview);
}

bool CoreResults::project_result(int job_id, int step, int totalincrement,double scale)
{
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);
  CoreResultsProject* projector = new CoreResultsProject();

  if (results_data_id == -1)
  {
    return false;
  } else {
    if (step!=-1)
    {
      log = "Project results for Job ID " + std::to_string(results_data[results_data_id][1]) + " for Step " + std::to_string(step) + " \n";
    }else{
      log = "Project results for Job ID " + std::to_string(results_data[results_data_id][1]) + " for total increment " + std::to_string(totalincrement) + " \n";
    }
    PRINT_INFO("%s", log.c_str());
    projector->init(&frd_data[frd_data_id]);
    if (!projector->project(step,totalincrement,scale))
    {
      delete projector;
      return false;
    }

    delete projector;

    return true;
  }
}

int CoreResults::get_results_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < results_data.size(); i++)
  {
    if (results_data[i][1]==job_id)
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

int CoreResults::get_frd_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < frd_data.size(); i++)
  {
    if (job_id == frd_data[i].job_id)
    {
      return_int = int(i);
    }  
  }
  
  return return_int;
}

int CoreResults::get_dat_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < dat_data.size(); i++)
  {
    if (job_id == dat_data[i].job_id)
    {
      return_int = int(i);
    }  
  }
  
  return return_int;
}

std::string CoreResults::print_data()
{
  std::string str_return;
  str_return = "\n CoreResults results_data: \n";
  str_return.append("result_id, job_id, frd_id, dat_id \n");

  for (size_t i = 0; i < results_data.size(); i++)
  {
    str_return.append(std::to_string(results_data[i][0]) + " " + std::to_string(results_data[i][1]) + " " + std::to_string(results_data[i][2]) + " " + std::to_string(results_data[i][3]) +  " \n");
  }

  return str_return;
}

std::string CoreResults::retained_identity(int job_id, std::string& error)
{
  error.clear();
  if (!can_change_readers())
  {
    error = "CalculiX reader data is busy.";
    return {};
  }
  CoreResultsVtkPlan plan;
  if (!prepare_vtk_plan_impl(job_id,CoreResultsVtkOptions{},plan))
  {
    error = "The saved reader/mesh data cannot reproduce this result.";
    return {};
  }
  const auto frd = get_frd_data_id_from_job_id(job_id), dat = get_dat_data_id_from_job_id(job_id);
  const auto identity = retained_result_identity(frd_data[frd],dat_data[dat],plan);
  const auto saved = retained_identities.find(job_id);
  if (saved != retained_identities.end() && saved->second != identity)
  {
    error = "Saved result content or mesh-link metadata no longer matches its identity.";
    return {};
  }
  return identity;
}
