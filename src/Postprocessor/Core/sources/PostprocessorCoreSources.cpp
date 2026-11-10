#include "../application/PostprocessorCore.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "../representations/ResultMesh.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <limits>
#include <utility>
#include <variant>
#include <set>

namespace
{
  // Provider callbacks may service host events. Keep read access available while
  // rejecting nested source mutations, and release the guard on every exit path.
  class SourceOperationGuard
  {
  public:
    explicit SourceOperationGuard(bool& active) : active(active)
    {
      active = true;
    }

    ~SourceOperationGuard()
    {
      active = false;
    }

    SourceOperationGuard(const SourceOperationGuard&) = delete;
    SourceOperationGuard& operator=(const SourceOperationGuard&) = delete;

  private:
    bool& active;
  };

  bool prepare_table(Postprocessor::PostprocessorSession& staged, const std::string& source,
    const std::shared_ptr<const SimResults::VtkResultsData>& data, std::string& error)
  {
    const auto tables = Postprocessor::result_tables(*data,std::nullopt);
    auto& view = staged.views.at(staged.active_view_id);
    view.table.reset();
    if (tables.empty())
    {
      return true;
    }
    Postprocessor::TableReference reference;
    reference.source_id = source;
    reference.key = tables.front().key;
    auto table = Postprocessor::result_table_data(data,reference);
    for (std::size_t index = 0; !table.table && index < data->increments.size(); ++index)
    {
      reference.frame = index;
      table = Postprocessor::result_table_data(data,reference);
    }
    if (!table.table || table.rows > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
      error = "The first result table is unavailable or exceeds the table view row limit.";
      return false;
    }
    view.table = std::move(reference);
    staged.active_representation_id.clear();
    return true;
  }

  int category(const SimResults::Dataset& dataset)
  {
    if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::Blocks))
    {
      return 0;
    }
    if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::NodeSets))
    {
      return 1;
    }
    if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::SideSets))
    {
      return 2;
    }
    return Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::IntegrationPoints) ? 3 : 4;
  }

  // New sources only: choose blocks, otherwise the first present category in
  // catalogue/tree order. Other representations and restored state are untouched.
  void initialize_result_display(Postprocessor::PostprocessorSession& staged,
    const SimResults::VtkResultsData& data, const std::vector<SimResults::Dataset>& datasets)
  {
    const auto active = staged.representations.find(staged.active_representation_id);
    if (active == staged.representations.end())
    {
      return;
    }
    auto& display = active->second;
    const auto& frame = data.increments.at(display.increment_index);
    const auto parts = Postprocessor::result_mesh_parts(frame.dataset.get());
    std::set<std::size_t> present;
    for (const auto& part : parts)
    {
      present.insert(part.first);
    }
    int selectedCategory = -1;
    for (const auto& dataset : datasets)
    {
      if (present.count(dataset.id) != 0)
      {
        const int kind = category(dataset);
        if (selectedCategory < 0 || kind == 0)
        {
          selectedCategory = kind;
        }
        if (kind == 0)
        {
          break;
        }
      }
    }
    for (const auto& dataset : datasets)
    {
      if (category(dataset) != selectedCategory)
      {
        display.hidden_datasets.insert(dataset.id);
      }
    }
    // result_field_groups uses the same name/key ordering as the result tree.
    // Select only fields with valid samples on the initially visible datasets.
    for (const auto& group : Postprocessor::result_field_groups(data,display.increment_index))
    {
      const auto& field = group.field;
      if (!group.available || field.association == SimResults::Association::Table || field.components.empty())
      {
        continue;
      }
      Postprocessor::FieldSelection selection;
      selection.array_name = field.array_name;
      selection.association = field.association;
      selection.group_key = group.key;
      selection.magnitude = field.vector_components.has_value();
      if (!selection.magnitude)
      {
        selection.component = field.components.front();
      }
      std::optional<std::array<double,2>> range;
      for (const auto& current : frame.fields)
      {
        if (!Postprocessor::dataset_visible(display,current.dataset_id) ||
            Postprocessor::field_group_key(current) != group.key)
        {
          continue;
        }
        for (const auto& part : parts)
        {
          if (part.first == current.dataset_id)
          {
            std::string issue;
            Postprocessor::accumulate_color_range(part.second,current,selection,range,issue);
          }
        }
      }
      if (range)
      {
        display.coloring.field = std::move(selection);
        break;
      }
    }
  }

  bool valid_name(const std::string& name)
  {
    return name.find_first_not_of(" \t\r\n\f\v") != std::string::npos;
  }

  bool validate_request(
    const ResultSourceProviderInterface& provider,
    const SimResults::ResultSourceReference& request,
    std::string& error)
  {
    if (provider.command_prefix().empty())
    {
      error = "The result provider has no command prefix.";
      return false;
    }

    if (const auto* job = std::get_if<SimResults::JobSourceReference>(&request))
    {
      if (job->application_id.empty() || job->job_id <= 0)
      {
        error = "A job result request requires an application identity and a positive job ID.";
        return false;
      }
    }
    else
    {
      const auto& files = std::get<SimResults::FileSourceReference>(request);

      if (files.provider_id != provider.command_prefix() || files.paths.empty())
      {
        error = "A file result request requires matching provider identity and source paths.";
        return false;
      }

      for (const auto& path : files.paths)
      {
        if (path.empty())
        {
          error = "A result source path cannot be empty.";
          return false;
        }
      }
    }

    return true;
  }

  bool validate_publication(
    const SimResults::ResultSourceReference& request,
    const SimResults::PublishedResultSource& published,
    bool refresh,
    std::string& error)
  {
    if (!published.data || (published.data->increments.empty() && published.data->tables.empty()))
    {
      error = "The result provider did not publish result states or tables.";
      return false;
    }

    if (const auto* requested = std::get_if<SimResults::JobSourceReference>(&request))
    {
      const auto* resolved = std::get_if<SimResults::JobSourceReference>(&published.reference);

      if (resolved == nullptr || resolved->application_id != requested->application_id ||
          resolved->project_id != requested->project_id || resolved->job_id != requested->job_id ||
          resolved->run_identity.empty() || published.data->job_id != resolved->job_id ||
          published.data->run_identity != resolved->run_identity ||
          (!refresh && !requested->run_identity.empty() && requested->run_identity != resolved->run_identity))
      {
        error = "The published result does not match the requested job snapshot.";
        return false;
      }
    }
    else
    {
      const auto& files = std::get<SimResults::FileSourceReference>(request);
      const auto* resolved = std::get_if<SimResults::FileSourceReference>(&published.reference);

      if (resolved == nullptr || resolved->provider_id != files.provider_id || resolved->paths.empty())
      {
        error = "The published file source does not match the requested provider.";
        return false;
      }

      for (const auto& path : resolved->paths)
      {
        if (path.empty() || !std::filesystem::u8path(path).is_absolute())
        {
          error = "Published result source paths must be resolved absolute paths.";
          return false;
        }
      }
    }

    return true;
  }
}

namespace Postprocessor
{
  // Committing staged definitions must not allocate or partially publish state.
  static_assert(noexcept(std::declval<PostprocessorSession&>().swap(std::declval<PostprocessorSession&>())),
    "Postprocessor session publication requires non-throwing swap.");

  bool PostprocessorCore::open_result(
    ResultSourceProviderInterface& provider,
    const SimResults::ResultSourceReference& request,
    const std::optional<std::string>& name,
    std::string& error)
  {
    error.clear();

    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "A Postprocessor source operation is active or display revisions are exhausted.";
      return false;
    }
    SourceOperationGuard guard(source_operation_active);

    try
    {
      if (name && (!valid_name(*name) || !source_id_from_name(*name).empty()))
      {
        error = "The requested source name is blank or already in use. Supply a distinct name.";
        return false;
      }

      if (!validate_request(provider, request, error))
      {
        return false;
      }

      SimResults::PublishedResultSource published;

      if (!provider.open(request, published, error))
      {
        if (error.empty())
        {
          error = "The result provider failed to open the source.";
        }
        return false;
      }

      if (!validate_publication(request, published, false, error))
      {
        return false;
      }

      // Opening an already retained job snapshot without a name selects it.
      // Explicit names still request independently retained sources. A newer
      // run never silently replaces an older source with the same job name.
      if (!name)
      {
        const auto* resolved = std::get_if<SimResults::JobSourceReference>(&published.reference);
        for (const auto& entry : state.sources)
        {
          const auto* existing = std::get_if<JobSourceReference>(&entry.second.reference);
          if (resolved && existing && existing->application_id == resolved->application_id &&
            existing->project_id == resolved->project_id && existing->job_id == resolved->job_id &&
            existing->run_identity == resolved->run_identity)
          {
            const auto loaded = runtime.find(entry.first);
            if (loaded != runtime.end() && loaded->second.data == published.data)
            {
              auto staged = state;
              if (staged.active_source_id != entry.first)
              {
                for (auto& display : staged.representations)
                {
                  if (display.second.view_id == staged.active_view_id)
                  {
                    display.second.animation.playing = false;
                  }
                }
              }
              staged.active_source_id = entry.first;
              staged.active_representation_id.clear();
              if (!prepare_initial_display(staged, entry.first, loaded->second.first_mesh_increment, error))
              {
                return false;
              }
              if (!loaded->second.first_mesh_increment && !prepare_table(staged,entry.first,published.data,error))
              {
                return false;
              }
              state.swap(staged);
              ++scene_revision;
              return true;
            }
          }
        }
      }

      const std::string resolved_name = name ? *name : published.suggested_name;

      if (!valid_name(resolved_name) || !source_id_from_name(resolved_name).empty())
      {
        error = "The result source name is blank or already in use. Supply a distinct name.";
        return false;
      }

      std::uint64_t number = next_source_number;
      std::string source_id;

      while (true)
      {
        if (number == std::numeric_limits<std::uint64_t>::max())
        {
          error = "Postprocessor source identifiers are exhausted.";
          return false;
        }

        source_id = "source" + std::to_string(number);

        if (state.sources.find(source_id) == state.sources.end())
        {
          break;
        }

        ++number;
      }

      DataSource source;
      source.id = source_id;
      source.name = resolved_name;
      source.reference = std::visit(
        [](const auto& reference) -> SourceReference
        {
          return reference;
        }, published.reference);

      const auto mesh_increment = first_mesh_increment(*published.data);
      auto staged_state = state;
      auto staged_runtime = runtime;
      staged_state.sources.emplace(source_id, std::move(source));
      staged_runtime.emplace(source_id, SourceRuntime{published.data, 1, mesh_increment});
      if (staged_state.active_source_id != source_id)
      {
        for (auto& display : staged_state.representations)
        {
          if (display.second.view_id == staged_state.active_view_id)
          {
            display.second.animation.playing = false;
          }
        }
      }
      staged_state.active_source_id = source_id;
      staged_state.active_representation_id.clear();
      if (!prepare_initial_display(staged_state, source_id, mesh_increment, error))
      {
        return false;
      }
      if (mesh_increment)
      {
        initialize_result_display(staged_state,*published.data,staged_runtime.at(source_id).datasets);
      }
      if (!mesh_increment && !prepare_table(staged_state,source_id,published.data,error))
      {
        return false;
      }
      state.swap(staged_state);
      runtime.swap(staged_runtime);
      next_source_number = number + 1;
      ++scene_revision;
      error.clear();
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot open Postprocessor source: ") + exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot open Postprocessor source: unknown failure.";
      return false;
    }
  }

  bool PostprocessorCore::refresh_result(
    ResultSourceProviderInterface& provider,
    const std::string& source_id,
    std::string& error)
  {
    error.clear();

    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "A Postprocessor source operation is active or display revisions are exhausted.";
      return false;
    }
    SourceOperationGuard guard(source_operation_active);

    try
    {
      const auto source = state.sources.find(source_id);
      const auto loaded = runtime.find(source_id);

      if (source == state.sources.end() || loaded == runtime.end())
      {
        error = "The Postprocessor result source is not loaded.";
        return false;
      }

      if (loaded->second.revision == std::numeric_limits<std::uint64_t>::max())
      {
        error = "The Postprocessor source revision is exhausted.";
        return false;
      }

      SimResults::ResultSourceReference request;

      if (const auto* job = std::get_if<JobSourceReference>(&source->second.reference))
      {
        request = *job;
      }
      else if (const auto* files = std::get_if<FileSourceReference>(&source->second.reference))
      {
        request = *files;
      }
      else
      {
        error = "Cubit geometry refresh requires the geometry interface.";
        return false;
      }

      if (!validate_request(provider, request, error))
      {
        return false;
      }

      SimResults::PublishedResultSource published;

      if (!provider.refresh(request, published, error))
      {
        if (error.empty())
        {
          error = "The result provider failed to refresh the source.";
        }
        return false;
      }

      if (!validate_publication(request, published, true, error))
      {
        return false;
      }

      const auto mesh_increment = first_mesh_increment(*published.data);
      auto staged_state = state;
      auto staged_runtime = runtime;
      staged_state.sources.at(source_id).reference = std::visit(
        [](const auto& reference) -> SourceReference
        {
          return reference;
        }, published.reference);
      staged_runtime.at(source_id) = SourceRuntime{published.data, loaded->second.revision + 1, mesh_increment};
      if (!clear_source_selection(staged_state,source_id,error))
      {
        return false;
      }
      for (auto display = staged_state.representations.begin(); display != staged_state.representations.end();)
      {
        auto& definition = display->second;
        if (definition.input.owner == OutputOwner::Source && definition.input.object_id == source_id)
        {
          definition.animation.playing = false;
          definition.hidden_datasets.clear();
          if (!mesh_increment)
          {
            if (staged_state.active_representation_id == display->first)
            {
              staged_state.active_representation_id.clear();
            }
            display = staged_state.representations.erase(display);
            continue;
          }
          if (definition.increment_index >= published.data->increments.size() ||
              result_meshes(published.data->increments[definition.increment_index].dataset.get()).empty())
          {
            definition.increment_index = *mesh_increment;
          }
        }
        ++display;
      }
      state.swap(staged_state);
      runtime.swap(staged_runtime);
      ++scene_revision;
      error.clear();
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot refresh Postprocessor source: ") + exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot refresh Postprocessor source: unknown failure.";
      return false;
    }
  }

  bool PostprocessorCore::select_source(const std::string& source_id, std::string& error)
  {
    error.clear();

    if (source_operation_active || state.sources.find(source_id) == state.sources.end())
    {
      error = "Cannot select a missing source or change selection during a source operation.";
      return false;
    }
    SourceOperationGuard guard(source_operation_active);

    try
    {
      std::string selected = source_id;
      if (state.active_source_id != source_id)
      {
        const bool running = std::any_of(state.representations.begin(),state.representations.end(),[this](const auto& entry)
        {
          return entry.second.view_id == state.active_view_id && entry.second.animation.playing;
        });
        if (running && scene_revision == std::numeric_limits<std::uint64_t>::max())
        {
          error = "Cannot stop animation after revision exhaustion.";
          return false;
        }
        for (auto& entry : state.representations)
        {
          if (entry.second.view_id == state.active_view_id)
          {
            entry.second.animation.playing = false;
          }
        }
        if (running)
        {
          ++scene_revision;
        }
      }
      auto& view = state.views.at(state.active_view_id);
      if (view.table && view.table->source_id != source_id)
      {
        if (scene_revision == std::numeric_limits<std::uint64_t>::max())
        {
          error = "Cannot change view after revision exhaustion.";
          return false;
        }
        view.table.reset();
        ++scene_revision;
      }
      state.active_source_id.swap(selected);
      state.active_representation_id.clear();
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot select Postprocessor source: ") + exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot select Postprocessor source: unknown failure.";
      return false;
    }
  }

  bool PostprocessorCore::close_source(const std::string& source_id, std::string& error)
  {
    error.clear();

    if (source_operation_active || state.sources.find(source_id) == state.sources.end() ||
        scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot close a missing source or close sources during a source operation.";
      return false;
    }
    SourceOperationGuard guard(source_operation_active);

    try
    {
      auto staged_state = state;
      auto staged_runtime = runtime;

      for (auto display = staged_state.representations.begin(); display != staged_state.representations.end();)
      {
        if (display->second.input.owner == OutputOwner::Source && display->second.input.object_id == source_id)
        {
          if (staged_state.active_representation_id == display->first)
          {
            staged_state.active_representation_id.clear();
          }
          display = staged_state.representations.erase(display);
        }
        else
        {
          ++display;
        }
      }

      if (staged_state.active_source_id == source_id)
      {
        staged_state.active_source_id.clear();
        staged_state.active_representation_id.clear();
      }

      staged_state.sources.erase(source_id);
      if (!clear_source_selection(staged_state,source_id,error))
      {
        return false;
      }
      staged_runtime.erase(source_id);
      state.swap(staged_state);
      runtime.swap(staged_runtime);
      ++scene_revision;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot close Postprocessor source: ") + exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot close Postprocessor source: unknown failure.";
      return false;
    }
  }

  bool PostprocessorCore::reset(std::string& error)
  {
    error.clear();
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot reset Postprocessor during a source operation or after revision exhaustion.";
      return false;
    }
    SourceOperationGuard guard(source_operation_active);
    try
    {
      PostprocessorSession staged;
      std::uint64_t cameraRevision = 0;
      std::uint64_t fitRequest = 0;
      std::uint64_t selectionRevision = 0;
      for (const auto& entry : state.views)
      {
        cameraRevision = std::max(cameraRevision, entry.second.camera_revision);
        fitRequest = std::max(fitRequest, entry.second.fit_request);
        selectionRevision = std::max(selectionRevision,entry.second.selection.revision);
      }
      if (cameraRevision == std::numeric_limits<std::uint64_t>::max() ||
          fitRequest == std::numeric_limits<std::uint64_t>::max() || selectionRevision == std::numeric_limits<std::uint64_t>::max())
      {
        error = "Cannot reset Postprocessor after camera or selection revision exhaustion.";
        return false;
      }
      auto& view = staged.views.at(staged.active_view_id);
      view.camera_revision = cameraRevision + 1;
      view.fit_request = fitRequest + 1;
      view.selection.revision = selectionRevision + 1;
      state.swap(staged);
      runtime.clear();
      saved_bindings.clear();
      saved_table_frames.clear();
      // Retain the source ID allocator. Old handles/tokens must not address a
      // newly opened source after reset. Published handles owned elsewhere live on.
      ++scene_revision;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot reset Postprocessor: ") + exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot reset Postprocessor: unknown failure.";
      return false;
    }
  }

  bool PostprocessorCore::close_all_sources(std::string& error)
  {
    error.clear();

    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot close sources during a Postprocessor source operation.";
      return false;
    }
    SourceOperationGuard guard(source_operation_active);

    if (!clear_source_selection(state,"",error))
    {
      return false;
    }
    state.representations.clear();
    state.sources.clear();
    runtime.clear();
    saved_bindings.clear();
    saved_table_frames.clear();
    state.active_source_id.clear();
    state.active_representation_id.clear();
    ++scene_revision;
    return true;
  }
}
