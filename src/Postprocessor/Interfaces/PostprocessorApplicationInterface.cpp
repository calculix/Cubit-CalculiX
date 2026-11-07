#include "PostprocessorApplicationInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"

PostprocessorApplicationInterface::PostprocessorApplicationInterface(ResultSourceRegistryInterface& providers)
  : providers(providers)
{
}

bool PostprocessorApplicationInterface::open_job(
  const std::string& prefix,
  int job_id,
  const std::optional<std::string>& name,
  std::string& error)
{
  error.clear();
  ResolvedResultSourceProvider resolved;

  if (!providers.resolve_result_provider(prefix, resolved, error))
  {
    return false;
  }

  if (resolved.provider == nullptr || resolved.application_id.empty())
  {
    error = "The result provider registration is incomplete.";
    return false;
  }

  SimResults::JobSourceReference request;
  request.application_id = resolved.application_id;
  request.job_id = job_id;
  return core.open_result(*resolved.provider, request, name, error);
}

const Postprocessor::PostprocessorSession& PostprocessorApplicationInterface::session() const noexcept
{
  return core.session();
}

bool PostprocessorApplicationInterface::select_source(const std::string& name, std::string& error)
{
  error.clear();
  const auto id = core.source_id_from_name(name);

  if (id.empty())
  {
    error = "Unknown Postprocessor source: " + name;
    return false;
  }

  return core.select_source(id, error);
}

bool PostprocessorApplicationInterface::close_source(const std::string& name, std::string& error)
{
  error.clear();
  const auto id = core.source_id_from_name(name);

  if (id.empty())
  {
    error = "Unknown Postprocessor source: " + name;
    return false;
  }

  return core.close_source(id, error);
}

bool PostprocessorApplicationInterface::close_all_sources(std::string& error)
{
  return core.close_all_sources(error);
}

bool PostprocessorApplicationInterface::initialize_user_options(UserOptionsInterface& store, std::string& error)
{
  return options.initialize(store, error);
}

void PostprocessorApplicationInterface::shutdown_user_options() noexcept
{
  options.detach();
}

const Postprocessor::UserOptions& PostprocessorApplicationInterface::user_options() const noexcept
{
  return options.current();
}

const std::string& PostprocessorApplicationInterface::active_icons_path() const noexcept
{
  return options.active_icons_path();
}

bool PostprocessorApplicationInterface::user_options_restart_required() const
{
  return options.restart_required();
}

bool PostprocessorApplicationInterface::apply_user_options(const UserOptionValues& changes, std::string& error)
{
  return options.apply(changes, error);
}

std::vector<UserOption> PostprocessorApplicationInterface::user_option_definitions() const
{
  return Postprocessor::PostprocessorUserOptions::definitions();
}

bool PostprocessorApplicationInterface::user_option_values(UserOptionValues& snapshot, std::string& error) const
{
  return options.values(snapshot, error);
}
