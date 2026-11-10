#include "../application/PostprocessorCore.hpp"
#include "../representations/ResultMesh.hpp"
#include <exception>
#include <limits>
#include <vtkStaticCellLinks.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkVariant.h>
#include <set>

namespace Postprocessor
{
  bool clear_source_selection(PostprocessorSession& session, const std::string& source_id, std::string& error)
  {
    error.clear();
    for (const auto& entry : session.views)
    {
      const auto& selection = entry.second.selection;
      if (selection.entity && (source_id.empty() || selection.entity->source_id == source_id) &&
          selection.revision == std::numeric_limits<std::uint64_t>::max())
      {
        error = "Selection revision identifiers are exhausted.";
        return false;
      }
    }
    for (auto& entry : session.views)
    {
      if (entry.second.table && (source_id.empty() || entry.second.table->source_id == source_id))
      {
        entry.second.table.reset();
      }
      auto& selection = entry.second.selection;
      if (selection.entity && (source_id.empty() || selection.entity->source_id == source_id))
      {
        selection.entity.reset();
        ++selection.revision;
      }
    }
    return true;
  }

  bool PostprocessorCore::set_pick_mode(const std::string& view_id, EntityKind kind, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (source_operation_active || view == state.views.end() ||
        (kind != EntityKind::Node && kind != EntityKind::Element && kind != EntityKind::IntegrationPoint) ||
        view->second.selection.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot change the pick mode for this view during the current operation.";
      return false;
    }
    auto& selection = view->second.selection;
    if (selection.mode != kind)
    {
      selection.mode = kind;
      ++selection.revision;
    }
    return true;
  }

  bool PostprocessorCore::clear_pick(const std::string& view_id, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (source_operation_active || view == state.views.end() ||
        view->second.selection.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot clear selection for this view during the current operation.";
      return false;
    }
    if (view->second.selection.entity)
    {
      view->second.selection.entity.reset();
      ++view->second.selection.revision;
    }
    return true;
  }

  bool PostprocessorCore::pick_entity(const std::string& view_id, const EntityReference& entity, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (source_operation_active || view == state.views.end() || state.sources.count(entity.source_id) == 0 ||
        (entity.kind != EntityKind::Node && entity.kind != EntityKind::Element && entity.kind != EntityKind::IntegrationPoint) ||
        entity.entity_id < 0 || (entity.kind == EntityKind::IntegrationPoint ?
          !entity.integration_point || *entity.integration_point <= 0 || *entity.integration_point > 2147483647 : entity.integration_point.has_value()) ||
        view->second.selection.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "The source/view or original entity identity is invalid for selection.";
      return false;
    }
    try
    {
      const auto data = result_data(entity.source_id);
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (display.view_id != view_id || display.input.owner != OutputOwner::Source ||
            display.input.object_id != entity.source_id || display.input.port != 0 || !data)
        {
          continue;
        }
        const auto result = probe_result(*data,display,entity);
        if (!result.available)
        {
          error = "The requested original entity ID is unavailable in this dataset/frame.";
          return false;
        }
        auto staged = state;
        auto& selection = staged.views.at(view_id).selection;
        if (selection.entity && same_entity(*selection.entity,entity) && selection.mode == entity.kind &&
            staged.active_source_id == entity.source_id && staged.active_representation_id == entry.first)
        {
          return true;
        }
        selection.entity = entity;
        selection.mode = entity.kind;
        ++selection.revision;
        bool stoppedAnimation = false;
        if (staged.active_source_id != entity.source_id)
        {
          for (auto& other : staged.representations)
          {
            if (other.second.view_id == view_id && other.second.animation.playing)
            {
              other.second.animation.playing = false;
              stoppedAnimation = true;
            }
          }
        }
        if (stoppedAnimation && scene_revision == std::numeric_limits<std::uint64_t>::max())
        {
          error = "Cannot stop animation after display revision exhaustion.";
          return false;
        }
        staged.active_source_id = entity.source_id;
        staged.active_representation_id = entry.first;
        state.swap(staged);
        if (stoppedAnimation)
        {
          ++scene_revision;
        }
        return true;
      }
      error = "The selected source has no representation in this view.";
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot select a result entity: ")+exception.what();
    }
    catch (...)
    {
      error = "Cannot select a result entity: unknown failure.";
    }
    return false;
  }

  std::optional<ProbeResult> PostprocessorCore::probe(const std::string& view_id, std::string& error) const
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (view == state.views.end() || !view->second.selection.entity)
    {
      error = "No result entity is selected in this view.";
      return std::nullopt;
    }
    return probe_entity(view_id,*view->second.selection.entity,error);
  }

  std::optional<ProbeResult> PostprocessorCore::probe_entity(const std::string& view_id,
    const EntityReference& entity, std::string& error) const
  {
    error.clear();
    if (state.views.find(view_id) == state.views.end())
    {
      error = "The probe view is unavailable.";
      return std::nullopt;
    }
    try
    {
      const auto data = result_data(entity.source_id);
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (display.view_id == view_id && display.input.owner == OutputOwner::Source &&
            display.input.object_id == entity.source_id && display.input.port == 0 && data)
        {
          auto result = probe_result(*data,display,entity);
          result.source_name = state.sources.at(entity.source_id).name;
          if (entity.kind == EntityKind::Node && display.increment_index < data->increments.size())
          {
            const auto source = runtime.find(entity.source_id);
            std::set<std::int64_t> parents;
            for (const auto& part : result_mesh_parts(data->increments[display.increment_index].dataset.get()))
            {
              auto* ids = part.second->GetCellData()->GetArray("OriginalElementId");
              if (!ids || ids->GetNumberOfComponents() != 1 || ids->GetDataType() == VTK_FLOAT || ids->GetDataType() == VTK_DOUBLE ||
                  part.second->GetPointData()->HasArray("IntegrationPointId") || source == runtime.end())
              {
                continue;
              }
              std::string issue;
              const auto points = entity_indices(part.second,entity,issue);
              if (points.empty())
              {
                continue;
              }
              auto& links = source->second.node_links[part.second];
              if (!links)
              {
                auto candidate = std::shared_ptr<vtkStaticCellLinks>(vtkStaticCellLinks::New(),[](vtkStaticCellLinks* value)
                {
                  value->Delete();
                });
                candidate->SetDataSet(part.second);
                candidate->BuildLinks();
                links = std::move(candidate);
              }
              for (auto point : points)
              {
                const auto count = links->GetNcells(static_cast<vtkIdType>(point));
                const auto* cells = links->GetCells(static_cast<vtkIdType>(point));
                for (vtkIdType index = 0; index < count; ++index)
                {
                  if (cells[index] < ids->GetNumberOfTuples())
                  {
                    bool valid = false;
                    const auto id = ids->GetVariantValue(cells[index]).ToLongLong(&valid);
                    if (valid && id >= 0)
                    {
                      parents.insert(id);
                    }
                  }
                }
              }
            }
            result.element_ids.assign(parents.begin(),parents.end());
          }
          return result;
        }
      }
      error = "The selected result source/representation is unavailable.";
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot probe the selected result: ")+exception.what();
    }
    catch (...)
    {
      error = "Cannot probe the selected result: unknown failure.";
    }
    return std::nullopt;
  }
}
