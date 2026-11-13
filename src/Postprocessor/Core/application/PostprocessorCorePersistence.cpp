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
      return data.increments.empty() ? std::optional<std::size_t>{0} : std::nullopt;
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
    return found == data.end() ? std::nullopt : std::optional<SavedDatasetReference>{{found->kind,found->name,found->model_id}};
  }
  std::optional<std::size_t> find_dataset(const std::vector<SimResults::Dataset>& data, const SavedDatasetReference& saved)
  {
    std::optional<std::size_t> found;
    for (const auto& item : data)
    {
      if (item.kind == saved.kind && (saved.model_id >= 0 ? item.model_id == saved.model_id : item.name == saved.name))
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
    const auto found = runtime.find(id);
    return found == runtime.end() ? "Result source is unresolved." : found->second.unavailable_reason;
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
        const auto loaded = runtime.find(item.first);
        if (loaded == runtime.end())
        {
          continue;
        }
        project.catalogues[item.first] = loaded->second.datasets;
        if (auto* job = std::get_if<JobSourceReference>(&item.second.reference))
        {
          if (loaded->second.data)
          {
            job->result_identity = loaded->second.data->result_identity;
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
          const auto saved = saved_table_frames.find(item.first);
          project.table_frames[item.first] = data ? frame_reference(*data,item.second.table->frame) :
            saved == saved_table_frames.end() ? SavedFrameReference{} : saved->second;
        }
      }
      for (auto& item : project.session.representations)
      {
        auto& display = item.second;
        display.animation.playing = false;
        const auto loaded = runtime.find(display.input.object_id);
        const auto previous = saved_bindings.find(item.first);
        auto binding = previous == saved_bindings.end() ? SavedRepresentationBinding{} : previous->second;
        if (loaded != runtime.end() && loaded->second.data)
        {
          const auto& data = *loaded->second.data;
          binding = {};
          binding.frame = frame_reference(data,display.increment_index);
          for (auto id : display.hidden_datasets)
          {
            const auto reference = dataset_reference(loaded->second.datasets,id);
            if (!reference)
            {
              error = "A hidden dataset no longer has a durable catalogue identity.";
              return false;
            }
            binding.hidden.push_back(*reference);
          }
          if (display.coloring.field && display.coloring.field->dataset_id)
          {
            binding.color_dataset = dataset_reference(loaded->second.datasets,*display.coloring.field->dataset_id);
            if (!binding.color_dataset)
            {
              error = "Color dataset identity is unavailable.";
              return false;
            }
          }
          if (display.deformation.field && display.deformation.field->dataset_id)
          {
            binding.deformation_dataset = dataset_reference(loaded->second.datasets,*display.deformation.field->dataset_id);
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
      auto& loaded = prepared.runtime.at(display.input.object_id);
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
      display.hidden_datasets.clear();
      bool valid = true;
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
      for (int index = 0; index < 2; ++index)
      {
        const auto& reference = index == 0 ? binding.color_dataset : binding.deformation_dataset;
        auto& selection = index == 0 ? display.coloring.field : display.deformation.field;
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
    for (const auto& source : runtime)
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
    state.swap(prepared.definitions.session);
    runtime.swap(prepared.runtime);
    saved_bindings.swap(prepared.definitions.bindings);
    saved_table_frames.swap(prepared.definitions.table_frames);
    ++scene_revision;
    return true;
  }
}
