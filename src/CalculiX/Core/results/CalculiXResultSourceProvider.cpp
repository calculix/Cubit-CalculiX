#include "CalculiXResultSourceProvider.hpp"
#include "CalculiXResultsInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "vtk/CoreResultsVtkStructs.hpp"

#include <exception>
#include <QFileInfo>
#include <QString>
#include "CoreResultsExternalSource.hpp"
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
    const auto* files = std::get_if<SimResults::FileSourceReference>(&request);
    return files && CoreResultsExternalSource().open(*files,output,error);
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
    return open(current,output,error);
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
    resolved.result_identity = data->result_identity;
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

bool CalculiXResultSourceProvider::restore(const SimResults::ResultSourceReference& request,
  SimResults::PublishedResultSource& output, std::string& error)
{
  error.clear();
  if (const auto* files = std::get_if<SimResults::FileSourceReference>(&request))
  {
    return CoreResultsExternalSource().open(*files,output,error,true);
  }
  const auto* job = std::get_if<SimResults::JobSourceReference>(&request);
  if (!job || job->application_id != "calculix" || job->job_id <= 0 || job->result_identity.empty() || !job->project_id.empty())
  {
    error = "The saved CalculiX reference has no supported durable result identity.";
    return false;
  }
  try
  {
    CalculiXResultsInterface results;
    JobsSettings settings;
    CalculiXJobsInterface jobs;
    if (!jobs.get_job_settings(job->job_id,settings,error) || jobs.get_job_status(job->job_id) == 1 || results.is_vtk_build_active())
    {
      error = "The saved CalculiX job is missing or busy.";
      return false;
    }
    auto data = results.get_vtk_result(job->job_id);
    if (!data || data->result_identity != job->result_identity)
    {
      const auto retained = results.retained_identity(job->job_id,error);
      if (retained.empty() || retained != job->result_identity)
      {
        if (error.empty())
        {
          error = "The retained CalculiX result differs from the saved result.";
        }
        return false;
      }
      // Never call load_result(): only readers restored from .cub are allowed.
      if (!results.build_vtk_result(job->job_id,CoreResultsVtkOptions{}))
      {
        error = "Cannot reconstruct VTK data from the saved reader buffers.";
        return false;
      }
      data = results.get_vtk_result(job->job_id);
    }
    if (!data || data->result_identity != job->result_identity)
    {
      error = "The reconstructed result failed durable identity validation.";
      return false;
    }
    auto resolved = *job;
    resolved.run_identity = data->run_identity;
    SimResults::PublishedResultSource candidate;
    candidate.reference = resolved;
    candidate.suggested_name = settings.name;
    candidate.data = std::move(data);
    output = std::move(candidate);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot restore CalculiX result: ")+exception.what();
    return false;
  }
}

bool CalculiXResultSourceProvider::available_jobs(std::vector<SimResults::ResultJobEntry>& output, std::string& error) const
{
  CalculiXJobsInterface jobs;
  CalculiXResultsInterface results;
  std::vector<SimResults::ResultJobEntry> entries;
  for (const auto& row : jobs.get_jobs_tree_data())
  {
    bool valid = false;
    const int id = row.empty() ? -1 : QString::fromStdString(row[0]).toInt(&valid);
    JobsSettings settings;
    if (!valid || id <= 0 || !jobs.get_job_settings(id,settings,error))
    {
      error = "Cannot obtain result job metadata.";
      return false;
    }
    SimResults::ResultJobEntry entry;
    entry.reference.application_id = "calculix";
    entry.reference.job_id = id;
    entry.command_prefix = "ccx";
    entry.application_name = "CalculiX";
    entry.job_name = settings.name;
    const bool busy = jobs.get_job_status(id) == 1 || results.is_vtk_build_active();
    const auto data = results.get_vtk_result(id);
    const auto job = jobs.get_job_data(id);
    const bool files = job.size() > 1 && QFileInfo::exists(QString::fromStdString(job[1]+".frd")) &&
      QFileInfo::exists(QString::fromStdString(job[1]+".dat"));
    entry.available = !busy && (data || files);
    entry.unavailable_reason = busy ? "Job or result conversion is running." : files || data ? "" : "Result files are not available.";
    entries.push_back(std::move(entry));
  }
  output = std::move(entries);
  error.clear();
  return true;
}
