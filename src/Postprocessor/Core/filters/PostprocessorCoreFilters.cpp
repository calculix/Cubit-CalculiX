#include "../application/PostprocessorCore.hpp"
#include <QUuid>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkCell.h>
#include "../representations/ResultMesh.hpp"
#include <limits>
#include <algorithm>
namespace Postprocessor
{
  std::string PostprocessorCore::output_source(const OutputReference& input) const
  {
    if (input.port != 0)
    {
      return {};
    }
    if (input.owner == OutputOwner::Source)
    {
      return input.object_id;
    }
    const auto found = state.filters.find(input.object_id);
    return found != state.filters.end() ? found->second.input.object_id : std::string{};
  }
  bool PostprocessorCore::display_matches_source(const Representation& display,const std::string& source,const std::string& view) const
  {
    if (display.view_id != view || output_source(display.input) != source)
    {
      return false;
    }
    const auto active = state.representations.find(state.active_representation_id);
    if (active != state.representations.end() && active->second.view_id == view && output_source(active->second.input) == source)
    {
      return display.id == active->first;
    }
    return display.input.owner == OutputOwner::Source;
  }
  bool PostprocessorCore::create_filter(FilterDefinition requested,const std::string& view,std::string& error)
  {
    error.clear();
    requested.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    requested.revision = 1;
    if (source_operation_active || state.filters.size() >= FilterDefaults::maximum_filters ||
        scene_revision == std::numeric_limits<std::uint64_t>::max() || !normalize_plane(requested.plane,error) ||
        !valid_filter(requested,error) || state.sources.count(requested.input.object_id) == 0 || state.views.count(view) == 0 ||
        std::any_of(state.filters.begin(),state.filters.end(),[&](const auto& item)
        {
          return item.second.name == requested.name;
        }))
    {
      if (error.empty())
      {
        error = "Filter name must be unique and source/view available; filter limit is 128.";
      }
      return false;
    }
    auto staged = state;
    auto input = std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& item)
    {
      return item.second.input.owner == OutputOwner::Source && item.second.input.object_id == requested.input.object_id && item.second.view_id == view;
    });
    if (input == staged.representations.end() || !source_has_mesh(requested.input.object_id))
    {
      error = "Choose an opened mesh source in this view before creating a filter.";
      return false;
    }
    auto representation = input->second;
    representation.id = "filter-display:"+view+":"+requested.id;
    representation.name = requested.name;
    representation.input = {OutputOwner::Feature,requested.id,0};
    representation.visible = true;
    representation.animation.playing = false;
    EvaluatedOutput evaluated;
    if (!evaluate_filter(requested,representation,result_data(requested.input.object_id),evaluated,error))
    {
      return false;
    }
    input->second.visible = false;
    input->second.animation.playing = false;
    staged.filters.emplace(requested.id,requested);
    staged.representations.emplace(representation.id,representation);
    staged.active_view_id = view;
    staged.active_source_id = requested.input.object_id;
    staged.active_representation_id = representation.id;
    staged.views.at(view).table.reset();
    staged.views.at(view).selection.entity.reset();
    std::map<std::string,CachedFilterOutput> nextCache;
    nextCache[representation.id] = {requested.revision,representation.increment_index,representation.deformation,
      evaluated.source,std::make_shared<EvaluatedOutput>(std::move(evaluated))};
    for (auto& entry : staged.representations)
    {
      if (entry.second.view_id == view)
      {
        entry.second.animation.playing = false;
      }
    }
    state.swap(staged);
    filter_cache.swap(nextCache);
    ++scene_revision;
    return true;
  }
  bool PostprocessorCore::change_filter(const std::string& name,const std::string& action,const std::string& view,
    const PlaneDefinition& requestedPlane,const std::string& value,std::string& error)
  {
    error.clear();
    const auto found = std::find_if(state.filters.begin(),state.filters.end(),[&](const auto& item)
    {
      return item.second.name == name;
    });
    if (source_operation_active || found == state.filters.end() || state.views.count(view) == 0 ||
        scene_revision == std::numeric_limits<std::uint64_t>::max() || found->second.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "The filter/view is unavailable during this operation.";
      return false;
    }
    auto staged = state;
    auto& definition = staged.filters.at(found->first);
    if (action == "plane")
    {
      auto plane = requestedPlane;
      if (!normalize_plane(plane,error))
      {
        return false;
      }
      definition.plane = plane;
    }
    else if (action == "keep" && definition.kind == FilterKind::Clip && (value == "positive" || value == "negative"))
    {
      definition.keep = value == "positive" ? ClipSide::Positive : ClipSide::Negative;
    }
    else if (action == "rename")
    {
      definition.name = value;
      if (!valid_filter(definition,error) || std::any_of(staged.filters.begin(),staged.filters.end(),[&](const auto& item)
        {
          return item.first != definition.id && item.second.name == value;
        }))
      {
        if (error.empty())
        {
          error = "The filter name is already in use.";
        }
        return false;
      }
    }
    else if (action != "show" && action != "hide" && action != "remove")
    {
      error = "Invalid filter operation.";
      return false;
    }
    ++definition.revision;
    if (action == "show" && std::none_of(staged.representations.begin(),staged.representations.end(),[&](const auto& item)
      {
        return item.second.input.owner == OutputOwner::Feature && item.second.input.object_id == definition.id && item.second.view_id == view;
      }))
    {
      const auto source = std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& item)
      {
        return item.second.input.owner == OutputOwner::Source && item.second.input.object_id == definition.input.object_id;
      });
      if (source == staged.representations.end())
      {
        error = "The filter source has no representation to copy.";
        return false;
      }
      auto display = source->second;
      display.id = "filter-display:"+view+":"+definition.id;
      display.view_id = view;
      display.name = definition.name;
      display.input = {OutputOwner::Feature,definition.id,0};
      display.visible = true;
      display.animation.playing = false;
      staged.representations.emplace(display.id,std::move(display));
    }
    for (auto item = staged.representations.begin(); item != staged.representations.end();)
    {
      auto& display = item->second;
      if (display.input.owner != OutputOwner::Feature || display.input.object_id != definition.id)
      {
        ++item;
        continue;
      }
      if (action == "remove")
      {
        if (staged.active_representation_id == item->first)
        {
          staged.active_representation_id.clear();
        }
        item = staged.representations.erase(item);
        continue;
      }
      display.name = definition.name;
      if ((action == "show" || action == "hide") && display.view_id == view)
      {
        display.visible = action == "show";
        if (display.visible)
        {
          staged.active_representation_id = display.id;
          staged.active_source_id = definition.input.object_id;
          staged.active_view_id = view;
        }
      }
      ++item;
    }
    std::map<std::string,CachedFilterOutput> nextCache;
    for (auto& item : staged.representations)
    {
      auto& display = item.second;
      if (action == "show" && display.view_id == view)
      {
        display.animation.playing = false;
      }
      if ((action == "plane" || action == "keep") && display.visible && display.input.owner == OutputOwner::Feature && display.input.object_id == definition.id)
      {
        const auto source = result_data(definition.input.object_id);
        if (source)
        {
          auto result = std::make_shared<EvaluatedOutput>();
          if (!evaluate_filter(definition,display,source,*result,error))
          {
            return false;
          }
          nextCache[display.id] = {definition.revision,display.increment_index,display.deformation,source,result};
        }
      }
    }
    if (action == "remove")
    {
      staged.filters.erase(found->first);
    }
    state.swap(staged);
    filter_cache.swap(nextCache);
    ++scene_revision;
    return true;
  }
  bool PostprocessorCore::evaluate_output(const Representation& display,std::shared_ptr<const EvaluatedOutput>& output,std::string& error) const
  {
    error.clear();
    const auto source = result_data(output_source(display.input));
    if (!source)
    {
      error = "The output source is unavailable.";
      return false;
    }
    if (display.input.owner == OutputOwner::Source)
    {
      auto result = std::make_shared<EvaluatedOutput>();
      result->source = result->data = source;
      result->source_id = display.input.object_id;
      output = std::move(result);
      return true;
    }
    const auto found = state.filters.find(display.input.object_id);
    if (found == state.filters.end())
    {
      error = "The filter output is unavailable.";
      return false;
    }
    const auto cached = filter_cache.find(display.id);
    if (cached != filter_cache.end() && cached->second.source == source && cached->second.revision == found->second.revision &&
        cached->second.frame == display.increment_index && same_deformation(cached->second.deformation,display.deformation))
    {
      output = cached->second.output;
      return true;
    }
    auto result = std::make_shared<EvaluatedOutput>();
    if (!evaluate_filter(found->second,display,source,*result,error))
    {
      return false;
    }
    for (auto item = filter_cache.begin(); item != filter_cache.end();)
    {
      const auto display = state.representations.find(item->first);
      if (display == state.representations.end() || !display->second.visible)
      {
        item = filter_cache.erase(item);
      }
      else
      {
        ++item;
      }
    }
    filter_cache[display.id] = {found->second.revision,display.increment_index,display.deformation,source,result};
    output = std::move(result);
    return true;
  }
  std::optional<std::array<double,3>> PostprocessorCore::filtered_entity_position(const Representation& display,const EntityReference& entity) const
  {
    if (!display.visible || !dataset_visible(display,entity.dataset_id))
    {
      return std::nullopt;
    }
    std::shared_ptr<const EvaluatedOutput> output;
    std::string error;
    if (!evaluate_output(display,output,error))
    {
      return std::nullopt;
    }
    for (const auto& part : output->parts)
    {
      if (part.dataset != entity.dataset_id)
      {
        continue;
      }
      auto* mesh = entity.kind == EntityKind::Element ? part.mesh.get() : static_cast<vtkDataSet*>(part.nodes.get());
      const auto indices = entity_indices(mesh,entity,error);
      if (!indices.empty())
      {
        std::array<double,3> position = {};
        if (entity.kind != EntityKind::Element)
        {
          mesh->GetPoint(static_cast<vtkIdType>(indices.front()),position.data());
        }
        else
        {
          auto* cell = mesh->GetCell(static_cast<vtkIdType>(indices.front()));
          if (!cell || cell->GetNumberOfPoints() == 0)
          {
            return std::nullopt;
          }
          for (vtkIdType i = 0; i < cell->GetNumberOfPoints(); ++i)
          {
            double point[3];
            mesh->GetPoint(cell->GetPointId(i),point);
            for (int axis = 0; axis < 3; ++axis)
            {
              position[axis] += point[axis]/cell->GetNumberOfPoints();
            }
          }
        }
        return position;
      }
    }
    return std::nullopt;
  }

  std::shared_ptr<vtkPolyData> PostprocessorCore::filtered_entity_edges(const Representation& display,const EntityReference& entity,std::string& error) const
  {
    const auto filter = state.filters.find(display.input.object_id);
    const auto data = result_data(entity.source_id);
    if (filter == state.filters.end() || !data || display.increment_index >= data->increments.size())
    {
      return {};
    }
    const auto& frame = data->increments[display.increment_index];
    for (const auto& part : result_mesh_parts(frame.dataset.get()))
    {
      if (part.first != entity.dataset_id)
      {
        continue;
      }
      auto indices = entity_indices(part.second,entity,error);
      if (indices.empty())
      {
        return {};
      }
      std::shared_ptr<vtkDataSet> moved;
      std::string warning;
      if (!deform_mesh(part.second,part.first,frame,display.deformation,moved,warning,error))
      {
        return {};
      }
      return filter_parent_edges(moved ? moved.get() : part.second,filter->second,frame,part.first,indices,error);
    }
    return {};
  }

}
