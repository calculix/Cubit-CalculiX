#include "PostprocessorApplicationInterface.hpp"
#include <algorithm>
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

bool PostprocessorApplicationInterface::set_pick_mode(Postprocessor::EntityKind kind, std::string& error)
{
  return core.set_pick_mode(core.session().active_view_id,kind,error);
}

bool PostprocessorApplicationInterface::clear_pick(std::string& error)
{
  return core.clear_pick(core.session().active_view_id,error);
}

std::optional<Postprocessor::ProbeResult> PostprocessorApplicationInterface::probe(std::string& error) const
{
  return core.probe(core.session().active_view_id,error);
}

bool PostprocessorApplicationInterface::pick_entity(const std::string& name, const DatasetSelector& selector,
  Postprocessor::EntityKind kind, std::int64_t id, std::optional<std::int64_t> ip, std::string& error)
{
  const auto source = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  const auto dataset = resolve_dataset(source,selector,error);
  return dataset && core.pick_entity(core.session().active_view_id,
    Postprocessor::EntityReference{source,*dataset,kind,id,ip},error);
}

std::optional<std::size_t> PostprocessorApplicationInterface::resolve_dataset(const std::string& source,
  const DatasetSelector& selector, std::string& error) const
{
  error.clear();
  std::optional<std::size_t> found;
  for (const auto& value : core.mesh_datasets(source))
  {
    const auto* id = std::get_if<std::size_t>(&selector);
    if (id ? value.id == *id : !value.name.empty() && value.name == std::get<std::string>(selector))
    {
      if (found)
      {
        error = "The dataset name is ambiguous in this source. Use its numeric ID.";
        return std::nullopt;
      }
      found = value.id;
    }
  }
  if (!found)
  {
    error = "No mesh dataset has that published ID or name in this source.";
  }
  return found;
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

std::optional<Postprocessor::AnimationDefinition> PostprocessorApplicationInterface::animation(const std::string& name,std::string& error) const
{
  error.clear();
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  for (const auto& entry : core.session().representations)
  {
    const auto& display = entry.second;
    if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.object_id == id &&
        display.input.port == 0 && display.view_id == core.session().active_view_id)
    {
      return display.animation;
    }
  }
  error = "The source has no representation in the active view.";
  return std::nullopt;
}
bool PostprocessorApplicationInterface::set_animation(const std::string& name,const Postprocessor::AnimationDefinition& animation,std::string& error)
{
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  return core.set_animation(id,core.session().active_view_id,animation,error);
}

std::optional<std::array<double,2>> PostprocessorApplicationInterface::all_frame_range(const std::string& source_id,
  const Postprocessor::FieldSelection& selection,std::string& error,
  const std::set<std::size_t>& hidden) const
{
  return core.all_frame_range(source_id,selection,error,hidden);
}

std::optional<Postprocessor::ResultColoring> PostprocessorApplicationInterface::coloring(const std::string& name, std::string& error) const
{
  error.clear();
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  for (const auto& entry : core.session().representations)
  {
    const auto& display = entry.second;
    if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.object_id == id &&
        display.input.port == 0 && display.view_id == core.session().active_view_id)
    {
      return display.coloring;
    }
  }
  error = "The source has no representation in the active view.";
  return std::nullopt;
}

bool PostprocessorApplicationInterface::set_coloring(const std::string& name,
  const Postprocessor::ResultColoring& value, std::string& error)
{
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  return core.set_coloring(id, core.session().active_view_id, value, error);
}

std::optional<Postprocessor::ResultDeformation> PostprocessorApplicationInterface::deformation(
  const std::string& name, std::string& error) const
{
  error.clear();
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  for (const auto& entry : core.session().representations)
  {
    const auto& display = entry.second;
    if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.object_id == id &&
        display.input.port == 0 && display.view_id == core.session().active_view_id)
    {
      return display.deformation;
    }
  }
  error = "The source has no representation in the active view.";
  return std::nullopt;
}

bool PostprocessorApplicationInterface::set_deformation(const std::string& name,
  const Postprocessor::ResultDeformation& value, std::string& error)
{
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  return core.set_deformation(id,core.session().active_view_id,value,error);
}

bool PostprocessorApplicationInterface::deform_auto(const std::string& name, std::string& error)
{
  auto settings = deformation(name,error);
  if (!settings)
  {
    return false;
  }
  settings->field_mode = Postprocessor::DeformationFieldMode::Automatic;
  return set_deformation(name,*settings,error);
}

bool PostprocessorApplicationInterface::deform_field(const std::string& name, const std::string& field_name,
  std::optional<std::size_t> dataset, std::string& error)
{
  auto settings = deformation(name,error);
  if (!settings)
  {
    return false;
  }
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  const auto data = core.result_data(id);
  for (const auto& entry : core.session().representations)
  {
    const auto& display = entry.second;
    if (display.input.owner != Postprocessor::OutputOwner::Source || display.input.object_id != id ||
        display.input.port != 0 || display.view_id != core.session().active_view_id ||
        !data || display.increment_index >= data->increments.size())
    {
      continue;
    }
    const SimResults::Field* match = nullptr;
    for (const auto& field : data->increments[display.increment_index].fields)
    {
      if ((field.name == field_name || field.array_name == field_name) &&
          (!dataset || field.dataset_id == *dataset) && Postprocessor::deformation_vector(field))
      {
        if (match && Postprocessor::field_group_key(*match) != Postprocessor::field_group_key(field))
        {
          error = "Ambiguous displacement fields. Use the exact array name and dataset qualifier.";
          return false;
        }
        match = &field;
      }
    }
    if (!match)
    {
      error = "No compatible point-associated displacement vector is available in this frame.";
      return false;
    }
    settings->field_mode = Postprocessor::DeformationFieldMode::Explicit;
    settings->field = Postprocessor::FieldSelection{dataset,match->array_name,match->association,
      {},true,Postprocessor::field_group_key(*match)};
    return set_deformation(name,*settings,error);
  }
  error = "The source has no current result frame.";
  return false;
}

bool PostprocessorApplicationInterface::color_field(const std::string& name, const std::string& field_name,
  const std::string& component, bool magnitude, std::optional<std::size_t> dataset, std::string& error)
{
  auto settings = coloring(name, error);
  if (!settings)
  {
    return false;
  }
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  const auto data = core.result_data(id);
  for (const auto& entry : core.session().representations)
  {
    const auto& display = entry.second;
    if (display.input.object_id != id || display.view_id != core.session().active_view_id ||
        display.input.owner != Postprocessor::OutputOwner::Source || display.input.port != 0 ||
        !data || display.increment_index >= data->increments.size())
    {
      continue;
    }
    const SimResults::Field* match = nullptr;
    for (const auto& field : data->increments[display.increment_index].fields)
    {
      if ((field.name == field_name || field.array_name == field_name) && (!dataset || field.dataset_id == *dataset))
      {
        if (match != nullptr && Postprocessor::field_group_key(*match) != Postprocessor::field_group_key(field))
        {
          error = "Ambiguous field variants. Use the exact array name and dataset qualifier.";
          return false;
        }
        match = &field;
      }
    }
    if (!match || match->association == SimResults::Association::Table ||
        (magnitude && !match->vector_components) ||
        (!magnitude && std::find(match->components.begin(), match->components.end(), component) == match->components.end()))
    {
      error = "The mesh field/component is unavailable; magnitude requires declared vector components.";
      return false;
    }
    settings->field = Postprocessor::FieldSelection{dataset, match->array_name, match->association, component, magnitude, Postprocessor::field_group_key(*match)};
    return set_coloring(name, *settings, error);
  }
  error = "The source has no current result frame.";
  return false;
}

bool PostprocessorApplicationInterface::set_frame(const std::string& source_name, std::size_t frame_number, std::string& error)
{
  const auto id = source_name.empty() ? core.session().active_source_id : core.source_id_from_name(source_name);
  if (id.empty() || frame_number == 0)
  {
    error = "Frame selection requires an opened source and a positive frame number.";
    return false;
  }
  return core.set_frame(id, core.session().active_view_id, frame_number - 1, error);
}

std::vector<Postprocessor::ResultFieldGroup> PostprocessorApplicationInterface::field_groups(
  const std::string& source_id,std::optional<std::size_t> current) const
{
  return core.field_groups(source_id,current);
}

bool PostprocessorApplicationInterface::frame_has_mesh(const std::string& source_id, std::size_t increment_index) const
{
  return core.frame_has_mesh(source_id, increment_index);
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

std::vector<SimResults::Dataset> PostprocessorApplicationInterface::mesh_datasets(const std::string& source_id) const
{
  return core.mesh_datasets(source_id);
}

bool PostprocessorApplicationInterface::set_dataset_visibility(const std::string& name,
  Postprocessor::DatasetVisibilityAction action, const std::optional<DatasetSelector>& selector, std::string& error)
{
  error.clear();
  const auto id = name.empty() ? core.session().active_source_id : core.source_id_from_name(name);
  std::optional<std::size_t> dataset;
  if (selector)
  {
    dataset = resolve_dataset(id,*selector,error);
    if (!dataset)
    {
      return false;
    }
  }
  return core.set_dataset_visibility(id,core.session().active_view_id,action,dataset,error);
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
  if (!options.initialize(store, error))
  {
    return false;
  }
  core.set_default_palette(options.current().resultPalette);
  return true;
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
  if (!options.apply(changes, error))
  {
    return false;
  }
  core.set_default_palette(options.current().resultPalette);
  return true;
}

std::vector<UserOption> PostprocessorApplicationInterface::user_option_definitions() const
{
  return Postprocessor::PostprocessorUserOptions::definitions();
}

bool PostprocessorApplicationInterface::user_option_values(UserOptionValues& snapshot, std::string& error) const
{
  return options.values(snapshot, error);
}
