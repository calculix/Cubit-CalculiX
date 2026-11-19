#include <type_traits>
#include "Postprocessor/Core/filters/FilterRuntime.hpp"
#include "Postprocessor/Core/sources/SourceProjectRuntime.hpp"
#include "PostprocessorCore.hpp"
#include "../persistence/PostprocessorSessionCodec.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <charconv>
namespace
{
  using namespace Postprocessor;
  SavedFrameReference frame_reference(const SimResults::VtkResultsData& data, std::size_t index)
  {
    SavedFrameReference result;
    if (index < data.increments.size())
    {
      const auto& frame = data.increments[index];
      result = {true,frame.step,frame.increment,frame.analysis_type,frame.mode,frame.subcase,frame.nodal_diameter,frame.time};
    }
    return result;
  }
  std::optional<std::size_t> find_frame(const SimResults::VtkResultsData& data, const SavedFrameReference& saved)
  {
    if (!saved.valid)
    {
      if (data.increments.empty()) {
        return std::optional<std::size_t>{0};
      } else {
        return std::nullopt;
      }
    }
    std::optional<std::size_t> found;
    for (std::size_t index = 0; index < data.increments.size(); ++index)
    {
      const auto& frame = data.increments[index];
      // Time is an identity consistency check, never nearest-frame selection.
      const double tolerance = 1e-12*std::max({1.0,std::abs(frame.time),std::abs(saved.time)});
      if (frame.step == saved.step && frame.increment == saved.increment && frame.analysis_type == saved.analysis_type &&
          frame.mode == saved.mode && frame.subcase == saved.subcase && frame.nodal_diameter == saved.nodal_diameter &&
          std::abs(frame.time-saved.time) <= tolerance)
      {
        if (found)
        {
          return std::nullopt;
        }
        found = index;
      }
    }
    return found;
  }
  std::optional<SavedDatasetReference> dataset_reference(const std::vector<SimResults::Dataset>& data, std::size_t id)
  {
    const auto found = std::find_if(data.begin(),data.end(),[&](const auto& item)
    {
      return item.id == id;
    });
    if (found == data.end()) {
      return std::nullopt;
    } else {
      return std::optional<SavedDatasetReference>{{found->kind,found->name,found->model_id}};
    }
  }
  std::optional<std::size_t> find_dataset(const std::vector<SimResults::Dataset>& data, const SavedDatasetReference& saved)
  {
    std::optional<std::size_t> found;
    for (const auto& item : data)
    {
      if (item.kind == saved.kind && ([&]() -> std::common_type_t<decltype(item.model_id == saved.model_id), decltype(item.name == saved.name)> {
        if (saved.model_id >= 0) {
          return item.model_id == saved.model_id;
        } else {
          return item.name == saved.name;
        }
      }()))
      {
        if (found)
        {
          return std::nullopt;
        }
        found = item.id;
      }
    }
    return found;
  }
  void unavailable(SourceRuntime& runtime, const std::string& reason)
  {
    runtime.data.reset();
    runtime.first_mesh_increment.reset();
    runtime.availability = SourceAvailability::Unavailable;
    runtime.unavailable_reason = reason;
    runtime.node_links.clear();
    runtime.all_frame_ranges.clear();
  }
}
namespace Postprocessor
{
  std::string PostprocessorCore::source_unavailable_reason(const std::string& id) const
  {
    const auto* found = source_runtime(id);
    if (!found) {
      return "Result source is unresolved.";
    } else {
      return found->unavailable_reason;
    }
  }
  bool PostprocessorCore::capture_project(PostprocessorProjectState& output, std::string& error) const
  {
    error.clear();
    if (source_operation_active)
    {
      error = "Cannot save Postprocessor definitions during a source operation.";
      return false;
    }
    try
    {
      PostprocessorProjectState project;
      project.session = state;
      for (auto& item : project.session.sources)
      {
        const auto* loaded = source_runtime(item.first);
        if (!loaded)
        {
          continue;
        }
        project.catalogues[item.first] = loaded->datasets;
        if (auto* job = std::get_if<JobSourceReference>(&item.second.reference))
        {
          if (loaded->data)
          {
            job->result_identity = loaded->data->result_identity;
          }
        }
      }
      for (auto& item : project.session.views)
      {
        item.second.selection.entity.reset();
        item.second.fit_pending = !item.second.camera.has_value();
        if (item.second.table)
        {
          const auto data = result_data(item.second.table->source_id);
          const auto saved = sourceProjectRuntime->table_frames().find(item.first);
          if (data) {
            project.table_frames[item.first] = frame_reference(*data,item.second.table->frame);
          } else {
            if (saved == sourceProjectRuntime->table_frames().end()) {
              project.table_frames[item.first] = SavedFrameReference{};
            } else {
              project.table_frames[item.first] = saved->second;
            }
          }
        }
      }
      for (auto& item : project.session.representations)
      {
        auto& display = item.second;
        display.animation.playing = false;
        const auto* loaded = source_runtime(output_source(display.input));
        const auto previous = sourceProjectRuntime->bindings().find(item.first);
        auto binding = [&]() -> std::common_type_t<decltype(SavedRepresentationBinding{}), decltype(previous->second)> {
          if (previous == sourceProjectRuntime->bindings().end()) {
            return SavedRepresentationBinding{};
          } else {
            return previous->second;
          }
        }();
        if (loaded && loaded->data)
        {
          const auto& data = *loaded->data;
          binding = {};
          binding.frame = frame_reference(data,display.increment_index);
          if (previous != sourceProjectRuntime->bindings().end())
          {
            for (const auto& saved : previous->second.appearance)
            {
              if (!find_dataset(loaded->datasets,saved.dataset))
              {
                binding.appearance.push_back(saved);
              }
            }
          }
          for (const auto& appearance : display.dataset_appearance)
          {
            const auto reference = dataset_reference(loaded->datasets,appearance.first);
            if (!reference)
            {
              error = "Appearance dataset identity is unavailable.";
              return false;
            }
            binding.appearance.push_back({*reference,appearance.second});
          }
          for (auto id : display.hidden_datasets)
          {
            const auto reference = dataset_reference(loaded->datasets,id);
            if (!reference)
            {
              error = "A hidden dataset no longer has a durable catalogue identity.";
              return false;
            }
            binding.hidden.push_back(*reference);
          }
          if (display.coloring.field && display.coloring.field->dataset_id)
          {
            binding.color_dataset = dataset_reference(loaded->datasets,*display.coloring.field->dataset_id);
            if (!binding.color_dataset)
            {
              error = "Color dataset identity is unavailable.";
              return false;
            }
          }
          if (display.vectors.field && display.vectors.field->dataset_id)
          {
            binding.vector_dataset = dataset_reference(loaded->datasets,*display.vectors.field->dataset_id);
            if (!binding.vector_dataset)
            {
              error = "Vector dataset identity is unavailable.";
              return false;
            }
          }
          if (display.deformation.field && display.deformation.field->dataset_id)
          {
            binding.deformation_dataset = dataset_reference(loaded->datasets,*display.deformation.field->dataset_id);
            if (!binding.deformation_dataset)
            {
              error = "Deformation dataset identity is unavailable.";
              return false;
            }
          }
        }
        project.bindings[item.first] = std::move(binding);
      }
      if (!validate_project(project,error))
      {
        return false;
      }
      output = std::move(project);
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot capture Postprocessor project: ")+exception.what();
      return false;
    }
  }
  bool PostprocessorCore::bind_project(PreparedPostprocessorProject& prepared, std::string& error) const
  {
    if (!validate_project(prepared.definitions,error))
    {
      return false;
    }
    for (const auto& source : prepared.definitions.session.sources)
    {
      if (prepared.runtime.count(source.first) == 0)
      {
        error = "A prepared project source has no resolution status.";
        return false;
      }
    }
    auto& project = prepared.definitions;
    for (auto& item : project.session.representations)
    {
      auto& display = item.second;
      const auto source = [&]() -> std::common_type_t<decltype(display.input.object_id), decltype(project.session.filters.at(display.input.object_id).input.object_id)> {
        if (display.input.owner == OutputOwner::Source) {
          return display.input.object_id;
        } else {
          return project.session.filters.at(display.input.object_id).input.object_id;
        }
      }();
      auto& loaded = prepared.runtime.at(source);
      display.animation.playing = false;
      if (!loaded.data)
      {
        continue;
      }
      const auto& binding = project.bindings.at(item.first);
      const auto frame = find_frame(*loaded.data,binding.frame);
      if (!frame)
      {
        unavailable(loaded,"The saved result frame is missing or ambiguous.");
        continue;
      }
      display.increment_index = *frame;
      auto& sceneUnit = project.session.views.at(display.view_id).scene_unit;
      if (sceneUnit.empty())
      {
        sceneUnit = filter_coordinate_unit(*loaded.data,display.increment_index);
      }

      display.hidden_datasets.clear();
      display.dataset_appearance.clear();
      bool valid = true;
      for (const auto& saved : binding.appearance)
      {
        const auto id = find_dataset(loaded.datasets,saved.dataset);
        if (!id)
        {
          valid = false;
          break;
        }
        const auto metadata = std::find_if(loaded.datasets.begin(),loaded.datasets.end(),[&](const auto& item)
        {
          return item.id == *id;
        });
        if (metadata == loaded.datasets.end() || !valid_appearance(*metadata,saved.appearance,error))
        {
          valid = false;
          break;
        }
        display.dataset_appearance[*id] = saved.appearance;
      }
      for (const auto& reference : binding.hidden)
      {
        const auto id = find_dataset(loaded.datasets,reference);
        if (!id)
        {
          valid = false;
          break;
        }
        display.hidden_datasets.insert(*id);
      }
      for (int index = 0; index < 3; ++index)
      {
        const auto& reference = [&]() -> decltype((binding.color_dataset)) {
          if (index == 0) {
            return binding.color_dataset;
          } else if (index == 2) {
            return binding.vector_dataset;
          } else {
            return binding.deformation_dataset;
          }
        }();
        auto& selection = [&]() -> decltype((display.coloring.field)) {
          if (index == 0) {
            return display.coloring.field;
          } else if (index == 2) {
            return display.vectors.field;
          } else {
            return display.deformation.field;
          }
        }();
        if (selection)
        {
          selection->dataset_id.reset();
          if (reference)
          {
            const auto id = find_dataset(loaded.datasets,*reference);
            if (!id)
            {
              valid = false;
              break;
            }
            selection->dataset_id = *id;
          }
        }
      }
      if (!valid)
      {
        unavailable(loaded,"A saved dataset is missing or ambiguous; display settings were retained.");
      }
    }
    for (auto& item : project.session.views)
    {
      auto& view = item.second;
      view.selection.entity.reset();
      view.fit_pending = !view.camera.has_value();
      if (view.table)
      {
        auto& loaded = prepared.runtime.at(view.table->source_id);
        const auto found = project.table_frames.find(item.first);
        if (loaded.data && found != project.table_frames.end())
        {
          const auto frame = find_frame(*loaded.data,found->second);
          if (frame)
          {
            view.table->frame = *frame;
          }
          else
          {
            unavailable(loaded,"The saved table frame is unavailable.");
          }
        }
      }
    }
    error.clear();
    return true;
  }
  bool PostprocessorCore::restore_project(PreparedPostprocessorProject&& prepared, std::string& error)
  {
    error.clear();
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        !validate_project(prepared.definitions,error))
    {
      if (error.empty())
      {
        error = "Cannot publish Postprocessor project during this operation.";
      }
      return false;
    }
    if (prepared.runtime.size() != prepared.definitions.session.sources.size())
    {
      error = "Prepared project runtime/source registries do not agree.";
      return false;
    }
    std::uint64_t next = next_source_number;
    for (const auto& source : prepared.definitions.session.sources)
    {
      if (prepared.runtime.count(source.first) == 0)
      {
        error = "A project source has no prepared runtime status.";
        return false;
      }
      if (source.first.rfind("source",0) == 0)
      {
        std::uint64_t number = 0;
        const auto* begin = source.first.data()+6;
        const auto* end = source.first.data()+source.first.size();
        const auto parsed = std::from_chars(begin,end,number);
        if (parsed.ec == std::errc{} && parsed.ptr == end)
        {
          if (number == std::numeric_limits<std::uint64_t>::max())
          {
            error = "Restored source identifiers exhaust the allocator.";
            return false;
          }
          next = std::max(next,number+1);
        }
      }
    }
    std::uint64_t sourceRevision = 0;
    for (const auto& source : sourceProjectRuntime->sources())
    {
      sourceRevision = std::max(sourceRevision,source.second.revision);
    }
    if (sourceRevision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Postprocessor source revision identifiers are exhausted.";
      return false;
    }
    for (auto& source : prepared.runtime)
    {
      source.second.revision = sourceRevision+1;
    }
    std::uint64_t token = 0;
    for (const auto& item : state.views)
    {
      token = std::max({token,item.second.camera_revision,item.second.fit_request,item.second.selection.revision,item.second.annotations.revision});
    }
    if (token == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Postprocessor view revisions are exhausted.";
      return false;
    }
    for (auto& item : prepared.definitions.session.views)
    {
      item.second.camera_revision = token+1;
      item.second.fit_request = token+1;
      item.second.selection.revision = token+1;
      item.second.annotations.revision = token+1;
      item.second.selection.entity.reset();
    }
    if (state.plot_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Plot revisions are exhausted.";
      return false;
    }
    prepared.definitions.session.plot_revision = state.plot_revision+1;
    for (auto& plot : prepared.definitions.session.plots)
    {
      plot.second.data_revision = prepared.definitions.session.plot_revision;
    }
    // Every allocation and validation is complete before publishing containers.
    next_source_number = next;
    invalidate_filter_runtime();
    invalidate_explosion_runtime();
    invalidate_animation_runtime();
    state.swap(prepared.definitions.session);
    sourceProjectRuntime->restore_project(prepared);
    ++scene_revision;
    return true;
  }
}
