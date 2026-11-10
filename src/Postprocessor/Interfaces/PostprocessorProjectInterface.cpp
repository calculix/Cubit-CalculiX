#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/persistence/PostprocessorProjectPersistence.hpp"
#include "Postprocessor/Core/sources/VtkFileSourceProvider.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <exception>
bool PostprocessorApplicationInterface::read_project_definitions(const std::string& filename, std::string& error)
{
  preparedProject.reset();
  auto candidate = std::make_unique<Postprocessor::PreparedPostprocessorProject>();
  if (!Postprocessor::PostprocessorProjectPersistence().read(filename,candidate->definitions,error))
  {
    return false;
  }
  preparedProject = std::move(candidate);
  return true;
}
bool PostprocessorApplicationInterface::resolve_project_sources(std::string& error)
{
  error.clear();
  if (!preparedProject)
  {
    error = "No Postprocessor project definitions have been prepared.";
    return false;
  }
  Postprocessor::VtkFileSourceProvider vtk;
  for (auto& item : preparedProject->definitions.session.sources)
  {
    const auto original = item.second.reference;
    std::string issue;
    ResultSourceProviderInterface* provider = nullptr;
    std::optional<SimResults::ResultSourceReference> request;
    ResolvedResultSourceProvider resolved;
    if (const auto* job = std::get_if<Postprocessor::JobSourceReference>(&original))
    {
      request = *job;
      if (providers.resolve_application_result_provider(job->application_id,resolved,issue))
      {
        provider = resolved.provider;
      }
    }
    else if (const auto* files = std::get_if<Postprocessor::FileSourceReference>(&original))
    {
      request = *files;
      if (files->provider_id == "vtk")
      {
        provider = &vtk;
      }
      else if (providers.resolve_result_provider(files->provider_id,resolved,issue))
      {
        provider = resolved.provider;
      }
    }
    else
    {
      issue = "Restoration of Cubit geometry sources is not implemented yet.";
    }
    SimResults::PublishedResultSource publication;
    bool available = false;
    try
    {
      available = provider && request && provider->restore(*request,publication,issue) && publication.data;
      if (available)
      {
        if (const auto* job = std::get_if<Postprocessor::JobSourceReference>(&original))
        {
          const auto* actual = std::get_if<SimResults::JobSourceReference>(&publication.reference);
          available = actual && actual->application_id == job->application_id && actual->project_id == job->project_id &&
            actual->job_id == job->job_id && !job->result_identity.empty() && actual->result_identity == job->result_identity &&
            publication.data->result_identity == job->result_identity;
        }
        else
        {
          const auto* files = std::get_if<Postprocessor::FileSourceReference>(&original);
          const auto* actual = std::get_if<SimResults::FileSourceReference>(&publication.reference);
          available = files && actual && actual->provider_id == files->provider_id &&
            !files->content_identity.empty() && files->content_identity == actual->content_identity;
        }
      }
    }
    catch (const std::exception& exception)
    {
      issue = exception.what();
      available = false;
    }
    Postprocessor::SourceRuntime runtime;
    if (available)
    {
      runtime = Postprocessor::SourceRuntime(publication.data,1,Postprocessor::first_mesh_increment(*publication.data));
      std::visit([&](const auto& reference)
      {
        item.second.reference = reference;
      },publication.reference);
    }
    else
    {
      runtime.availability = Postprocessor::SourceAvailability::Unavailable;
      runtime.unavailable_reason = issue.empty() ? "The saved result reference could not be verified." : issue;
      const auto catalogue = preparedProject->definitions.catalogues.find(item.first);
      if (catalogue != preparedProject->definitions.catalogues.end())
      {
        runtime.datasets = catalogue->second;
      }
    }
    preparedProject->runtime[item.first] = std::move(runtime);
  }
  if (!core.bind_project(*preparedProject,error))
  {
    return false;
  }
  for (auto& item : preparedProject->runtime)
  {
    if (!item.second.data)
    {
      const auto saved = preparedProject->definitions.catalogues.find(item.first);
      if (saved != preparedProject->definitions.catalogues.end())
      {
        item.second.datasets = saved->second;
      }
    }
  }
  return true;
}
bool PostprocessorApplicationInterface::commit_project_restore(std::string& error)
{
  if (!preparedProject)
  {
    error = "No Postprocessor project restore is prepared.";
    return false;
  }
  if (!core.restore_project(std::move(*preparedProject),error))
  {
    return false;
  }
  preparedProject.reset();
  return true;
}
bool PostprocessorApplicationInterface::prepare_project_save(std::string& error)
{
  auto candidate = std::make_unique<Postprocessor::PostprocessorProjectState>();
  if (!core.capture_project(*candidate,error))
  {
    return false;
  }
  preparedSave = std::move(candidate);
  return true;
}
bool PostprocessorApplicationInterface::write_project(const std::string& filename, std::string& error) const
{
  if (!preparedSave)
  {
    error = "Postprocessor project save was not prepared.";
    return false;
  }
  return Postprocessor::PostprocessorProjectPersistence().write(filename,*preparedSave,error);
}
void PostprocessorApplicationInterface::discard_project_restore() noexcept
{
  preparedProject.reset();
  preparedSave.reset();
}
std::string PostprocessorApplicationInterface::source_unavailable_reason(const std::string& id) const
{
  return core.source_unavailable_reason(id);
}
