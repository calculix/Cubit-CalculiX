#include <type_traits>
#include "../application/PostprocessorCore.hpp"
#include <QUuid>
#include <cmath>
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkCell.h>
#include "../representations/ResultMesh.hpp"
#include <limits>
#include <algorithm>
namespace Postprocessor
{
  bool PostprocessorCore::filter_motion_range(const std::string& name,const std::string& view,
    std::array<double,2>& range,std::string& error) const
  {
    error.clear();
    const auto filter = std::find_if(state.filters.begin(),state.filters.end(),[&](const auto& entry)
    {
      return entry.second.name == name;
    });
    if (filter == state.filters.end())
    {
      error = "The selected plane filter is unavailable.";
      return false;
    }
    const auto display = std::find_if(state.representations.begin(),state.representations.end(),[&](const auto& entry)
    {
      return entry.second.view_id == view && entry.second.input.owner == OutputOwner::Feature &&
        entry.second.input.object_id == filter->first;
    });
    const auto source = result_data(filter->second.input.object_id);
    if (display == state.representations.end() || !source || display->second.increment_index >= source->increments.size())
    {
      error = "The filter input frame is unavailable.";
      return false;
    }
    const auto& frame = source->increments[display->second.increment_index];
    if (!filter->second.coordinate_unit.empty())
    {
      double scale = 0;
      const auto unit = filter_coordinate_unit(*source,display->second.increment_index);
      if (unit.empty() || !framework_units().convert(1.0,filter->second.coordinate_unit,unit,scale,error) || std::abs(scale-1.0) > 1e-12)
      {
        error = "The plane and current source coordinate bases differ.";
        return false;
      }
    }
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -minimum;
    for (const auto& part : result_mesh_parts(frame.dataset.get()))
    {
      if (!dataset_visible(display->second,part.first) || part.second->GetNumberOfPoints() == 0)
      {
        continue;
      }
      std::shared_ptr<vtkDataSet> moved;
      std::string warning;
      if (!deform_mesh(part.second,part.first,frame,display->second.deformation,moved,warning,error))
      {
        return false;
      }
      auto* mesh = part.second;
      if (moved)
      {
        mesh = moved.get();
      }
      double bounds[6];
      mesh->GetBounds(bounds);
      // A linear plane functional reaches its box extrema at corner points.
      for (unsigned int corner = 0; corner < 8; ++corner)
      {
        std::array<double,3> point;
        for (unsigned int axis = 0; axis < 3; ++axis)
        {
          point[axis] = bounds[2*axis+((corner >> axis)&1)];
        }
        const double distance = signed_distance(filter->second.plane,point);
        minimum = std::min(minimum,distance);
        maximum = std::max(maximum,distance);
      }
    }
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum || !std::isfinite(maximum-minimum))
    {
      error = "The input has no finite extent along the plane normal.";
      return false;
    }
    range = {minimum,maximum};
    return true;
  }

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
    if (found != state.filters.end()) {
      return found->second.input.object_id;
    } else {
      return std::string{};
    }
  }
  bool PostprocessorCore::display_matches_source(const Representation& display,const std::string& source,const std::string& view) const
  {
    if (display.view_id != view || output_source(display.input) != source)
    {
      return false;
    }
    const auto active = state.representations.find(state.active_representation_id);
    const bool activeMatches = active != state.representations.end() && active->second.view_id == view &&
      output_source(active->second.input) == source;
    if (activeMatches && active->second.visible)
    {
      return display.id == active->first;
    }
    for (const auto& item : state.representations)
    {
      if (item.second.view_id == view && item.second.visible && output_source(item.second.input) == source)
      {
        return display.id == item.first;
      }
    }
    if (activeMatches)
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
    const auto sourceData = result_data(requested.input.object_id);
    if (sourceData)
    {
      requested.coordinate_unit = filter_coordinate_unit(*sourceData,representation.increment_index);
    }
    EvaluatedOutput evaluated;
    if (!evaluate_filter(requested,representation,result_data(requested.input.object_id),evaluated,error))
    {
      return false;
    }
    representation.dataset_appearance.clear();
    // One output of this source is shown per view when activating a filter.
    for (auto& item : staged.representations)
    {
      auto& other = item.second;
      if (other.view_id == view && output_source(other.input) == requested.input.object_id)
      {
        other.visible = false;
        other.animation.playing = false;
      }
    }
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
    const bool wasVisible = std::any_of(state.representations.begin(),state.representations.end(),[&](const auto& item)
    {
      return item.second.view_id == view && item.second.visible && item.second.input.owner == OutputOwner::Feature &&
        item.second.input.object_id == definition.id;
    });
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
      if (value == "positive") {
        definition.keep = ClipSide::Positive;
      } else {
        definition.keep = ClipSide::Negative;
      }
    }
    else if (action == "enable" || action == "disable")
    {
      definition.enabled = action == "enable";
    }
    else if (action == "reference" && definition.kind == FilterKind::Clip && (value == "on" || value == "off"))
    {
      definition.reference_visible = value == "on";
    }
    else if (action == "select")
    {
      const auto display = std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& item)
      {
        return item.second.view_id == view && item.second.input.owner == OutputOwner::Feature &&
          item.second.input.object_id == definition.id;
      });
      if (display == staged.representations.end())
      {
        error = "The filter has no representation in this view.";
        return false;
      }
      staged.active_source_id = definition.input.object_id;
      staged.active_representation_id = display->first;
      staged.active_view_id = view;
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
      display.dataset_appearance.clear();
      display.visible = true;
      display.animation.playing = false;
      staged.representations.emplace(display.id,std::move(display));
    }
    if (action == "show")
    {
      for (auto& item : staged.representations)
      {
        auto& other = item.second;
        if (other.view_id == view && output_source(other.input) == definition.input.object_id)
        {
          other.visible = other.input.owner == OutputOwner::Feature && other.input.object_id == definition.id;
          other.animation.playing = false;
        }
      }
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
    if (action == "hide" && wasVisible)
    {
      const bool anotherVisible = std::any_of(staged.representations.begin(),staged.representations.end(),[&](const auto& item)
      {
        return item.second.view_id == view && item.second.visible && output_source(item.second.input) == definition.input.object_id;
      });
      if (!anotherVisible)
      {
        for (auto& item : staged.representations)
        {
          auto& sourceDisplay = item.second;
          if (sourceDisplay.view_id == view && sourceDisplay.input.owner == OutputOwner::Source &&
              sourceDisplay.input.object_id == definition.input.object_id && sourceDisplay.input.port == 0)
          {
            sourceDisplay.visible = true;
            sourceDisplay.animation.playing = false;
            staged.active_representation_id = item.first;
            staged.active_source_id = definition.input.object_id;
            staged.active_view_id = view;
            break;
          }
        }
      }
    }
    std::map<std::string,CachedFilterOutput> nextCache;
    for (auto& item : staged.representations)
    {
      auto& display = item.second;
      if (action == "show" && display.view_id == view)
      {
        display.animation.playing = false;
      }
      if ((action == "plane" || action == "keep" || action == "enable" || action == "disable" || action == "reference") && display.visible && display.input.owner == OutputOwner::Feature && display.input.object_id == definition.id)
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
      auto* mesh = [&]() -> std::common_type_t<decltype(part.mesh.get()), decltype(static_cast<vtkDataSet*>(part.nodes.get()))> {
        if (entity.kind == EntityKind::Element) {
          return part.mesh.get();
        } else {
          return static_cast<vtkDataSet*>(part.nodes.get());
        }
      }();
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
      return filter_parent_edges([&]() -> std::common_type_t<decltype(moved.get()), decltype(part.second)> {
        if (moved) {
          return moved.get();
        } else {
          return part.second;
        }
      }(),filter->second,frame,part.first,indices,error);
    }
    return {};
  }

}
