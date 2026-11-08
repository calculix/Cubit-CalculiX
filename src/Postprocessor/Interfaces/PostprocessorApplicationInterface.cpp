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

bool PostprocessorApplicationInterface::reset(std::string& error)
{
  return core.reset(error);
}

bool PostprocessorApplicationInterface::close_all_sources(std::string& error)
{
  return core.close_all_sources(error);
}

bool PostprocessorApplicationInterface::set_source_visible(const std::string& name, bool visible, std::string& error)
{
  const auto id = core.source_id_from_name(name);
  if (id.empty())
  {
    error = "Unknown Postprocessor source: " + name;
    return false;
  }
  return core.set_source_visible(id, core.session().active_view_id, visible, error);
}

bool PostprocessorApplicationInterface::request_camera_fit(std::string& error)
{
  return core.request_camera_fit(core.session().active_view_id, error);
}

bool PostprocessorApplicationInterface::camera_navigation_ready(const std::string& view_id) const noexcept
{
  return core.camera_navigation_ready(view_id);
}

bool PostprocessorApplicationInterface::set_camera(const Postprocessor::CameraDefinition& camera, std::string& error)
{
  return core.set_camera(core.session().active_view_id, camera, error);
}

bool PostprocessorApplicationInterface::rotate_camera(double yaw, double pitch, std::string& error)
{
  return core.rotate_camera(core.session().active_view_id, yaw, pitch, error);
}

bool PostprocessorApplicationInterface::pan_camera(double horizontal, double vertical, std::string& error)
{
  return core.pan_camera(core.session().active_view_id, horizontal, vertical, error);
}

bool PostprocessorApplicationInterface::zoom_camera(double factor, std::string& error)
{
  return core.zoom_camera(core.session().active_view_id, factor, error);
}

std::shared_ptr<const SimResults::VtkResultsData> PostprocessorApplicationInterface::result_data(const std::string& source_id) const
{
  return core.result_data(source_id);
}

bool PostprocessorApplicationInterface::source_has_mesh(const std::string& source_id) const noexcept
{
  return core.source_has_mesh(source_id);
}

std::uint64_t PostprocessorApplicationInterface::display_revision() const noexcept
{
  return core.display_revision();
}

bool PostprocessorApplicationInterface::visible_bounds(const std::string& view_id, std::array<double, 6>& bounds, std::string& error) const
{
  return core.visible_bounds(view_id, bounds, error);
}

bool PostprocessorApplicationInterface::complete_camera_fit(const std::string& view_id, std::uint64_t request,
  std::uint64_t revision, const Postprocessor::CameraDefinition& camera, std::string& error)
{
  return core.complete_camera_fit(view_id, request, revision, camera, error);
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
