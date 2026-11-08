#include "CalculiXResultSourceProvider.hpp"
#include "CalculiXResultsInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "vtk/CoreResultsVtkStructs.hpp"

#include <exception>
#include <utility>
#include <variant>

std::string CalculiXResultSourceProvider::command_prefix() const
{
  return "ccx";
}

bool CalculiXResultSourceProvider::open(
  const SimResults::ResultSourceReference& request,
  SimResults::PublishedResultSource& output,
  std::string& error)
{
  error.clear();
  const auto* job = std::get_if<SimResults::JobSourceReference>(&request);

  if (job == nullptr)
  {
    error = "Independent CalculiX file loading is not implemented yet.";
    return false;
  }

  return load_job(*job, false, output, error);
}

bool CalculiXResultSourceProvider::refresh(
  const SimResults::ResultSourceReference& current,
  SimResults::PublishedResultSource& output,
  std::string& error)
{
  error.clear();
  const auto* job = std::get_if<SimResults::JobSourceReference>(&current);

  if (job == nullptr)
  {
    error = "Independent CalculiX file refresh is not implemented yet.";
    return false;
  }

  return load_job(*job, true, output, error);
}

bool CalculiXResultSourceProvider::load_job(
  const SimResults::JobSourceReference& request,
  bool refresh,
  SimResults::PublishedResultSource& output,
  std::string& error)
{
  if (request.application_id != "calculix" || request.job_id <= 0)
  {
    error = "CalculiX result loading requires application calculix and a positive job ID.";
    return false;
  }

  if (!request.project_id.empty())
  {
    error = "Project-qualified CalculiX result loading is not supported yet.";
    return false;
  }

  try
  {
    CalculiXJobsInterface jobs;
    CalculiXResultsInterface results;
    JobsSettings settings;

    if (!jobs.get_job_settings(request.job_id, settings, error))
    {
      if (error.empty())
      {
        error = "Cannot find CalculiX job " + std::to_string(request.job_id) + ".";
      }
      return false;
    }

    if (settings.name.empty())
    {
      error = "The CalculiX job has no result source name.";
      return false;
    }

    if (jobs.get_job_status(request.job_id) == 1 || results.is_vtk_build_active())
    {
      error = "Cannot open CalculiX results while the job or a VTK conversion is running.";
      return false;
    }

    std::shared_ptr<const SimResults::VtkResultsData> data;

    if (!refresh && !request.run_identity.empty())
    {
      data = results.get_vtk_result(request.job_id);

      // The existing token identifies a conversion snapshot, not a durable solver
      // run. Never rebuild current files and claim they match a historical token.
      if (!data || data->job_id != request.job_id || data->run_identity != request.run_identity)
      {
        error = "The requested CalculiX result snapshot is no longer retained.";
        return false;
      }
    }
    else if (!refresh && results.get_vtk_status(request.job_id) == SimResults::ConversionStatus::Complete)
    {
      data = results.get_vtk_result(request.job_id);
    }
    else
    {
      if (!results.load_result(request.job_id))
      {
        error = "Cannot load result files for CalculiX job " + std::to_string(request.job_id) + ".";
        return false;
      }

      if (!results.build_vtk_result(request.job_id, CoreResultsVtkOptions{}))
      {
        error = results.get_vtk_status(request.job_id) == SimResults::ConversionStatus::Cancelled
          ? "CalculiX result conversion was cancelled."
          : "Cannot convert results for CalculiX job " + std::to_string(request.job_id) + ".";
        return false;
      }

      data = results.get_vtk_result(request.job_id);
    }

    if (!data || data->job_id != request.job_id || data->run_identity.empty())
    {
      error = "CalculiX did not publish a valid result snapshot.";
      return false;
    }

    SimResults::PublishedResultSource published;
    SimResults::JobSourceReference resolved = request;
    resolved.application_id = "calculix";
    resolved.run_identity = data->run_identity;
    published.reference = std::move(resolved);
    published.suggested_name = settings.name;
    published.data = std::move(data);
    output = std::move(published);
    error.clear();
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("CalculiX result loading failed: ") + exception.what();
    return false;
  }
  catch (...)
  {
    error = "CalculiX result loading failed with an unknown exception.";
    return false;
  }
}
