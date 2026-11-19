#include <type_traits>
#include "Postprocessor/Core/persistence/PostprocessorProjectState.hpp"
#include "Postprocessor/Core/filters/FilterRuntime.hpp"
#include <vtkCell.h>
#include "../application/PostprocessorCore.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <utility>
#include "../representations/ResultMesh.hpp"
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkNew.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <locale>
#include <iomanip>

namespace Postprocessor
{
  bool valid_annotation_position(const std::array<double,2>& position)
  {
    return std::all_of(position.begin(),position.end(),[](double value)
    {
      return std::isfinite(value) && value >= 0 && value <= 1;
    });
  }
  bool valid_annotation_name(const std::string& name)
  {
    return !name.empty() && name.size() <= 256 && name.front() != '@' &&
      (name.find('"') == std::string::npos || name.find('\'') == std::string::npos) &&
      std::none_of(name.begin(),name.end(),[](unsigned char value)
      {
        return value < 32 || value == 127;
      });
  }
  ProbeResult annotation_probe(const SimResults::VtkResultsData& data, const Representation& display,
    const EntityReference& entity, const SimUnits::WorkspaceUnitContext* presentation)
  {
    auto result = probe_result(data,display,entity,presentation);
    if (result.displayed_position && !std::all_of(result.displayed_position->begin(),result.displayed_position->end(),[](double value)
      {
        return std::isfinite(value);
      }))
    {
      result.displayed_position.reset();
    }
    if (!result.available || entity.kind != EntityKind::Element || display.increment_index >= data.increments.size())
    {
      return result;
    }
    const auto& frame = data.increments[display.increment_index];
    std::optional<std::array<double,3>> anchor;
    for (const auto& part : result_mesh_parts(frame.dataset.get()))
    {
      if (part.first != entity.dataset_id)
      {
        continue;
      }
      std::string error;
      for (auto index : entity_indices(part.second,entity,error))
      {
        vtkNew<vtkIdList> points;
        part.second->GetCellPoints(static_cast<vtkIdType>(index),points);
        if (points->GetNumberOfIds() == 0)
        {
          continue;
        }
        std::array<double,3> mean = {0,0,0};
        for (vtkIdType node = 0; node < points->GetNumberOfIds(); ++node)
        {
          const auto position = deformed_position(part.second,static_cast<std::size_t>(points->GetId(node)),
            part.first,frame,display.deformation);
          for (std::size_t axis = 0; axis < 3; ++axis)
          {
            mean[axis] += position[axis]/static_cast<double>(points->GetNumberOfIds());
          }
        }
        if (!std::all_of(mean.begin(),mean.end(),[](double value)
          {
            return std::isfinite(value);
          }) || (anchor && *anchor != mean))
        {
          // Conflicting duplicated cells have no unique leader anchor.
          result.displayed_position.reset();
          return result;
        }
        anchor = mean;
      }
    }
    result.displayed_position = anchor;
    return result;
  }
  bool PostprocessorCore::add_label(const std::string& view_id, const ResultAnnotation& requested,
    std::string& assigned_name, std::string& error)
  {
    error.clear();
    assigned_name.clear();
    const auto view = state.views.find(view_id);
    if (source_operation_active || view == state.views.end() ||
        view->second.annotations.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Annotations cannot be changed during this operation.";
      return false;
    }
    auto label = requested;
    if (state.sources.count(label.entity.source_id) == 0 || label.entity.entity_id < 0 ||
        (label.entity.kind != EntityKind::Node && label.entity.kind != EntityKind::Element && label.entity.kind != EntityKind::IntegrationPoint) ||
        ([&]() -> std::common_type_t<decltype(!label.entity.integration_point ||
          *label.entity.integration_point < 1 || *label.entity.integration_point > 2147483647), decltype(label.entity.integration_point.has_value())> {
          if (label.entity.kind == EntityKind::IntegrationPoint) {
            return !label.entity.integration_point ||
          *label.entity.integration_point < 1 || *label.entity.integration_point > 2147483647;
          } else {
            return label.entity.integration_point.has_value();
          }
        }()))
    {
      error = "The original label entity identity is invalid.";
      return false;
    }
    if (label.name.empty())
    {
      for (std::size_t number = 1; number <= view->second.annotations.labels.size()+1; ++number)
      {
        const auto name = "label_"+std::to_string(number);
        if (view->second.annotations.labels.count(name) == 0)
        {
          label.name = name;
          break;
        }
      }
    }
    if (view->second.annotations.labels.size() >= PersistenceDefaults::maximum_records ||
        !valid_annotation_name(label.name) || !valid_annotation_position(label.position) ||
        view->second.annotations.labels.count(label.name) != 0)
    {
      error = "Label name must be unique, nonempty and not reserved; position must lie in [0,1].";
      return false;
    }
    const auto data = result_data(label.entity.source_id);
    bool available = false;
    for (const auto& entry : state.representations)
    {
      auto display = entry.second;
      if (display_matches_source(display,label.entity.source_id,view_id) && data)
      {
        display.coloring.field = label.field;
        const auto value = annotation_probe(*data,display,label.entity,captured_units());
        available = value.available && value.values.has_value() && value.displayed_position.has_value();
        break;
      }
    }
    if (!available)
    {
      error = "The label entity or requested field/component is unavailable in this frame.";
      return false;
    }
    bool found = false;
    for (const auto& dataset : mesh_datasets(label.entity.source_id))
    {
      if (dataset.id == label.entity.dataset_id)
      {
        label.dataset_kind = dataset.kind;
        label.dataset_name = dataset.name;
        label.model_id = dataset.model_id;
        found = true;
        break;
      }
    }
    if (!found)
    {
      error = "The label dataset is unavailable.";
      return false;
    }
    auto staged = state;
    auto& annotations = staged.views.at(view_id).annotations;
    // Stagger initial text positions; later positions are explicit commands.
    if (label.position == AnnotationDefaults::position)
    {
      const double offset = 0.08*static_cast<double>(annotations.labels.size()%5);
      label.position[1] -= offset;
    }
    annotations.labels.emplace(label.name,label);
    ++annotations.revision;
    assigned_name = label.name;
    state.swap(staged);
    return true;
  }
  bool PostprocessorCore::change_label(const std::string& view_id, const std::string& name,
    const std::string& action, const std::string& replacement, const std::array<double,2>& position, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (source_operation_active || view == state.views.end() ||
        view->second.annotations.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Annotations cannot be changed during this operation.";
      return false;
    }
    const auto& current = view->second.annotations;
    const bool extrema = name == "@minimum" || name == "@maximum";
    if ((extrema && action != "position") || (!extrema && current.labels.count(name) == 0) ||
        (action == "position" && !valid_annotation_position(position)) ||
        (action == "rename" && (!valid_annotation_name(replacement) ||
          (replacement != name && current.labels.count(replacement) != 0))) ||
        (action != "position" && action != "rename" && action != "show" && action != "hide" && action != "remove"))
    {
      error = "Unknown label/action, invalid position or conflicting label name.";
      return false;
    }
    auto staged = state;
    auto& annotations = staged.views.at(view_id).annotations;
    if (extrema)
    {
      if (name == "@minimum") {
        annotations.minimum_position = position;
      } else {
        annotations.maximum_position = position;
      }
    }
    else if (action == "remove")
    {
      annotations.labels.erase(name);
    }
    else if (action == "rename")
    {
      auto label = annotations.labels.at(name);
      label.name = replacement;
      annotations.labels.erase(name);
      annotations.labels.emplace(replacement,std::move(label));
    }
    else
    {
      auto& label = annotations.labels.at(name);
      if (action == "position")
      {
        label.position = position;
      }
      else
      {
        label.visible = action == "show";
      }
    }
    ++annotations.revision;
    state.swap(staged);
    return true;
  }
  bool PostprocessorCore::set_labels(const std::string& view_id, bool extrema, bool visible, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (source_operation_active || view == state.views.end() ||
        view->second.annotations.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Annotations cannot be changed during this operation.";
      return false;
    }
    auto& annotations = view->second.annotations;
    if (extrema) {
      annotations.extrema_visible = visible;
    } else {
      annotations.visible = visible;
    }
    ++annotations.revision;
    return true;
  }
  std::vector<EvaluatedAnnotation> PostprocessorCore::annotations(const std::string& view_id) const
  {
    auto values = native_annotations(view_id);
    const auto view = state.views.find(view_id);
    if (view == state.views.end())
    {
      return values;
    }
    for (auto& value : values)
    {
      if (!value.anchor)
      {
        continue;
      }
      std::string source;
      const auto label = view->second.annotations.labels.find(value.name);
      if (label != view->second.annotations.labels.end())
      {
        source = label->second.entity.source_id;
      }
      else
      {
        auto active = state.representations.find(state.active_representation_id);
        if (active == state.representations.end() || active->second.view_id != view_id)
        {
          active = std::find_if(state.representations.begin(),state.representations.end(),[&](const auto& item)
          {
            return item.second.view_id == view_id && item.second.input.owner == OutputOwner::Source &&
              item.second.visible && item.second.coloring.field.has_value();
          });
        }
        if (active != state.representations.end())
        {
          source = output_source(active->second.input);
        }
      }
      for (const auto& entry : state.representations)
      {
        if (entry.second.view_id != view_id || output_source(entry.second.input) != source)
        {
          continue;
        }
        double factor = 1;
        bool known = false;
        std::string error;
        if (scene_scale(entry.second,factor,known,error))
        {
          for (auto& coordinate : *value.anchor)
          {
            coordinate *= factor;
          }
        }
        else
        {
          value.anchor.reset();
        }
        break;
      }
    }
    return values;
  }
  std::vector<EvaluatedAnnotation> PostprocessorCore::native_annotations(const std::string& view_id) const
  {
    std::vector<EvaluatedAnnotation> output;
    const auto view = state.views.find(view_id);
    if (view == state.views.end() || !view->second.annotations.visible)
    {
      return output;
    }
    const auto& definitions = view->second.annotations;
    for (const auto& entry : definitions.labels)
    {
      auto label = entry.second;
      if (!label.visible)
      {
        continue;
      }
      bool hiddenByFilter = false;
      EvaluatedAnnotation value{label.name,"Unavailable",std::nullopt,label.position};
      value.text = std::string([&]() -> const char* {
        if (label.entity.kind == EntityKind::Node) {
          return "Node ";
        } else {
          return "Element ";
        }
      }())+std::to_string(label.entity.entity_id);
      if (label.entity.integration_point)
      {
        value.text += " / IP "+std::to_string(*label.entity.integration_point);
      }
      value.text += "\n"+label.field.array_name+" / "+([&]() -> std::common_type_t<decltype("Magnitude"), decltype(label.field.component)> {
        if (label.field.magnitude) {
          return "Magnitude";
        } else {
          return label.field.component;
        }
      }())+": Unavailable";
      const auto data = result_data(label.entity.source_id);
      // Resolve a durable dataset identity after refresh/restore, never reuse an
      // ordinal that may now refer to a different block or set.
      std::optional<std::size_t> resolved;
      bool ambiguous = false;
      for (const auto& dataset : mesh_datasets(label.entity.source_id))
      {
        if (dataset.kind == label.dataset_kind && ([&]() -> std::common_type_t<decltype(dataset.model_id == label.model_id), decltype(dataset.name == label.dataset_name)> {
          if (label.model_id >= 0) {
            return dataset.model_id == label.model_id;
          } else {
            return dataset.name == label.dataset_name;
          }
        }()))
        {
          if (resolved)
          {
            ambiguous = true;
            break;
          }
          resolved = dataset.id;
        }
      }
      if (resolved && !ambiguous)
      {
        label.entity.dataset_id = *resolved;
        label.field.dataset_id = *resolved;
      }
      if (data && resolved && !ambiguous)
      {
        for (const auto& item : state.representations)
        {
          auto display = item.second;
          if (display_matches_source(display,label.entity.source_id,view_id) && dataset_visible(display,label.entity.dataset_id))
          {
            display.coloring.field = label.field;
            auto result = annotation_probe(*data,display,label.entity,captured_units());
            if (display.input.owner == OutputOwner::Feature || explosion_active(display))
            {
              result.displayed_position = filtered_entity_position(display,label.entity);
              if (!result.displayed_position)
              {
                hiddenByFilter = true;
                continue;
              }
            }
            value.anchor = result.displayed_position;
            std::ostringstream text;
            text.imbue(std::locale::classic());
            text.precision(6);
            text << ([&]() -> const char* {
              if (label.entity.kind == EntityKind::Node) {
                return "Node ";
              } else {
                return "Element ";
              }
            }()) << label.entity.entity_id;
            if (label.entity.integration_point)
            {
              text << " / IP " << *label.entity.integration_point;
            }
            text << '\n' << result.field << " / " << result.component << ": ";
            if (result.values)
            {
              text << (*result.values)[0];
              if (result.range_value)
              {
                text << " .. " << (*result.values)[1];
              }
              if (!result.value_unit.empty())
              {
                text << " [" << result.value_unit << "]";
              }
            }
            else
            {
              text << "Unavailable";
            }
            value.text = text.str();
            break;
          }
        }
      }
      if (!hiddenByFilter)
      {
        output.push_back(std::move(value));
      }
    }
    if (!definitions.extrema_visible)
    {
      return output;
    }
    auto active = state.representations.find(state.active_representation_id);
    if (active == state.representations.end() || active->second.view_id != view_id)
    {
      active = std::find_if(state.representations.begin(),state.representations.end(),[&](const auto& item)
      {
        return item.second.view_id == view_id && item.second.input.owner == OutputOwner::Source &&
          item.second.input.object_id == state.active_source_id;
      });
    }
    if (active == state.representations.end() || active->second.view_id != view_id || !active->second.coloring.field)
    {
      return output;
    }
    const auto& display = active->second;
    if (display.input.owner == OutputOwner::Feature || explosion_active(display))
    {
      if (!display.visible)
      {
        return output;
      }
      std::shared_ptr<const EvaluatedOutput> evaluated;
      std::string error;
      if (!evaluate_output(display,evaluated,error))
      {
        return output;
      }
      const auto& frame = evaluated->data->increments[display.increment_index];
      std::optional<double> extrema[2];
      std::string extremaUnit;
      std::array<double,3> positions[2];
      for (const auto& part : result_mesh_parts(frame.dataset.get()))
      {
        if (!dataset_visible(display,part.first) || (display.coloring.field->dataset_id && *display.coloring.field->dataset_id != part.first))
        {
          continue;
        }
        auto selected = *display.coloring.field;
        selected.dataset_id = part.first;
        const auto* metadata = find_field(frame,selected);
        std::vector<double> values;
        if (!metadata || !color_values(part.second,*metadata,selected,values,error,captured_units()))
        {
          continue;
        }
        for (std::size_t i = 0; i < values.size(); ++i)
        {
          if (!std::isfinite(values[i]))
          {
            continue;
          }
          std::array<double,3> position = {};
          if (metadata->association == SimResults::Association::Element)
          {
            auto* cell = part.second->GetCell(static_cast<vtkIdType>(i));
            for (vtkIdType node = 0; cell && node < cell->GetNumberOfPoints(); ++node)
            {
              double point[3];
              part.second->GetPoint(cell->GetPointId(node),point);
              for (int axis = 0; axis < 3; ++axis)
              {
                position[axis] += point[axis]/cell->GetNumberOfPoints();
              }
            }
          }
          else
          {
            part.second->GetPoint(static_cast<vtkIdType>(i),position.data());
          }
          for (int index = 0; index < 2; ++index)
          {
            if (!extrema[index] || ([&]() -> std::common_type_t<decltype(values[i] < *extrema[index]), decltype(values[i] > *extrema[index])> {
              if (index == 0) {
                return values[i] < *extrema[index];
              } else {
                return values[i] > *extrema[index];
              }
            }()))
            {
              extrema[index] = values[i];
              positions[index] = position;
            }
          }
        }
      }
      for (int index = 0; index < 2; ++index)
      {
        if (extrema[index])
        {
          const auto* metadata = find_field(frame,*display.coloring.field);
          double ignored = 0.0;
          std::string issue;
          if (!metadata || !SimResults::ResultsUnits::field_value(0.0,*metadata,
                display.coloring.field->component,display.coloring.field->magnitude,
                "postprocessor",ignored,extremaUnit,issue,captured_units()))
          {
            continue;
          }
          std::ostringstream text;
          text.imbue(std::locale::classic());
          text << std::setprecision(6) << ([&]() -> const char* {
            if (index == 0) {
              return "Minimum: ";
            } else {
              return "Maximum: ";
            }
          }()) << *extrema[index] << " [" << extremaUnit << "]"
            << "\nFiltered display sample" << ([&]() -> const char* {
              if (display.coloring.field->association == SimResults::Association::Node) {
                return " (interpolated)";
              } else {
                return "";
              }
            }());
          output.push_back({[&]() -> const char* {
            if (index == 0) {
              return "@minimum";
            } else {
              return "@maximum";
            }
          }(),text.str(),positions[index],
            [&]() -> std::common_type_t<decltype(definitions.minimum_position), decltype(definitions.maximum_position)> {
              if (index == 0) {
                return definitions.minimum_position;
              } else {
                return definitions.maximum_position;
              }
            }()});
        }
      }
      return output;
    }
    const auto data = result_data(display.input.object_id);
    if (!data || display.increment_index >= data->increments.size())
    {
      return output;
    }
    const auto& frame = data->increments[display.increment_index];
    const auto& field = *display.coloring.field;
    std::optional<double> minimum, maximum;
    std::optional<EntityReference> minEntity, maxEntity;
    // Catalogue/tuple order breaks ties deterministically. No tessellation vertices.
    auto parts = result_mesh_parts(frame.dataset.get());
    std::sort(parts.begin(),parts.end(),[](const auto& a,const auto& b)
    {
      return a.first < b.first;
    });
    for (const auto& part : parts)
    {
      if (!dataset_visible(display,part.first) || (field.dataset_id && *field.dataset_id != part.first))
      {
        continue;
      }
      auto scoped = field;
      scoped.dataset_id = part.first;
      const auto* metadata = find_field(frame,scoped);
      if (!metadata)
      {
        continue;
      }
      const auto kind = [&]() -> std::common_type_t<decltype(EntityKind::Node), std::common_type_t<decltype(EntityKind::Element), decltype(EntityKind::IntegrationPoint)>> {
        if (field.association == SimResults::Association::Node) {
          return EntityKind::Node;
        } else {
          return [&]() -> std::common_type_t<decltype(EntityKind::Element), decltype(EntityKind::IntegrationPoint)> {
            if (field.association == SimResults::Association::Element) {
              return EntityKind::Element;
            } else {
              return EntityKind::IntegrationPoint;
            }
          }();
        }
      }();
      const auto count = [&]() -> std::common_type_t<decltype(part.second->GetNumberOfCells()), decltype(part.second->GetNumberOfPoints())> {
        if (kind == EntityKind::Element) {
          return part.second->GetNumberOfCells();
        } else {
          return part.second->GetNumberOfPoints();
        }
      }();
      for (vtkIdType index = 0; index < count; ++index)
      {
        std::string error;
        double sample = 0;
        EntityReference entity;
        if (!color_sample(part.second,*metadata,field,static_cast<std::size_t>(index),sample,error,captured_units()) || !std::isfinite(sample) ||
            !entity_from_index(part.second,static_cast<std::size_t>(index),kind,display.input.object_id,part.first,entity,error))
        {
          continue;
        }
        if (!minimum || sample < *minimum)
        {
          minimum = sample;
          minEntity = entity;
        }
        if (!maximum || sample > *maximum)
        {
          maximum = sample;
          maxEntity = entity;
        }
      }
    }
    for (int index = 0; index < 2; ++index)
    {
      const auto& entity = [&]() -> decltype((minEntity)) {
        if (index == 0) {
          return minEntity;
        } else {
          return maxEntity;
        }
      }();
      if (entity)
      {
        const auto probe = annotation_probe(*data,display,*entity,captured_units());
        std::ostringstream text;
        text.imbue(std::locale::classic());
        text.precision(6);
        text << ([&]() -> const char* {
          if (index == 0) {
            return "Minimum: ";
          } else {
            return "Maximum: ";
          }
        }()) << ([&]() -> std::common_type_t<decltype(*minimum), decltype(*maximum)> {
          if (index == 0) {
            return *minimum;
          } else {
            return *maximum;
          }
        }()) << " [" << probe.value_unit << "]"
          << '\n' << ([&]() -> const char* {
            if (entity->kind == EntityKind::Node) {
              return "Node ";
            } else {
              return "Element ";
            }
          }()) << entity->entity_id;
        if (entity->integration_point)
        {
          text << " / IP " << *entity->integration_point;
        }
        output.push_back({[&]() -> const char* {
          if (index == 0) {
            return "@minimum";
          } else {
            return "@maximum";
          }
        }(),text.str(),probe.displayed_position,
          [&]() -> std::common_type_t<decltype(definitions.minimum_position), decltype(definitions.maximum_position)> {
            if (index == 0) {
              return definitions.minimum_position;
            } else {
              return definitions.maximum_position;
            }
          }()});
      }
    }
    return output;
  }
}
