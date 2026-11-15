#include "ResultProbe.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "../representations/ResultMesh.hpp"
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <set>
#include <sstream>

namespace
{
  bool original_id(vtkDataArray* array, std::size_t index, std::int64_t& value) {
    if (!array || array->GetNumberOfComponents() != 1 || index >= static_cast<std::size_t>(array->GetNumberOfTuples()) ||
        array->GetDataType() == VTK_FLOAT || array->GetDataType() == VTK_DOUBLE) {
      return false;
    }
    bool valid = false;
    value = array->GetVariantValue(static_cast<vtkIdType>(index)).ToLongLong(&valid);
    return valid && value >= 0;
  }
}

namespace Postprocessor
{
  bool entity_from_index(vtkDataSet* mesh, std::size_t index, EntityKind kind,
    const std::string& source_id, std::size_t dataset, EntityReference& entity, std::string& error) {
    error.clear();
    if (!mesh || (kind != EntityKind::Node && kind != EntityKind::Element && kind != EntityKind::IntegrationPoint)) {
      error = "A supported entity kind and mesh are required.";
      return false;
    }
    vtkDataSetAttributes* attributes = mesh->GetPointData();
    if (kind == EntityKind::Element) {
      attributes = mesh->GetCellData();
    }
    const char* idName = "OriginalElementId";
    if (kind == EntityKind::Node) {
      idName = "OriginalNodeId";
    }
    std::int64_t id = 0, ip = 0;
    if ((kind == EntityKind::Element && mesh->GetPointData()->HasArray("IntegrationPointId")) ||
        !original_id(attributes->GetArray(idName),index,id) ||
        (kind == EntityKind::IntegrationPoint && (!original_id(attributes->GetArray("IntegrationPointId"),index,ip) || ip == 0 || ip > 2147483647))) {
      error = "Original result IDs are unavailable for this entity kind; rendering indices are not solver IDs.";
      return false;
    }
    std::optional<std::int64_t> integrationPoint;
    if (kind == EntityKind::IntegrationPoint) {
      integrationPoint = ip;
    }
    entity = EntityReference{source_id, dataset, kind, id, integrationPoint};
    return true;
  }

  std::vector<std::size_t> entity_indices(vtkDataSet* mesh, const EntityReference& entity, std::string& error) {
    error.clear();
    std::vector<std::size_t> result;
    if (!mesh) {
      return result;
    }
    if ((entity.kind != EntityKind::Node && entity.kind != EntityKind::Element && entity.kind != EntityKind::IntegrationPoint) ||
        (entity.kind == EntityKind::Element && mesh->GetPointData()->HasArray("IntegrationPointId"))) {
      error = "The entity kind is incompatible with this result dataset.";
      return result;
    }
    vtkDataSetAttributes* attributes = mesh->GetPointData();
    if (entity.kind == EntityKind::Element) {
      attributes = mesh->GetCellData();
    }
    const char* idName = "OriginalElementId";
    if (entity.kind == EntityKind::Node) {
      idName = "OriginalNodeId";
    }
    auto* ids = attributes->GetArray(idName);
    auto* ipIds = attributes->GetArray("IntegrationPointId");
    auto count = mesh->GetNumberOfPoints();
    if (entity.kind == EntityKind::Element) {
      count = mesh->GetNumberOfCells();
    }
    for (vtkIdType i = 0; i < count; ++i) {
      std::int64_t id = 0, ip = 0;
      if (!original_id(ids,static_cast<std::size_t>(i),id) || id != entity.entity_id ||
          (entity.kind == EntityKind::IntegrationPoint && (!entity.integration_point ||
            !original_id(ipIds,static_cast<std::size_t>(i),ip) || ip != *entity.integration_point))) {
        continue;
      }
      result.push_back(static_cast<std::size_t>(i));
    }
    if (!result.empty()) {
      error.clear();
    }
    return result;
  }

  ProbeResult probe_result(const SimResults::VtkResultsData& data, const Representation& display,
    const EntityReference& entity) {
    ProbeResult result;
    result.entity = entity;
    result.visible = dataset_visible(display,entity.dataset_id);
    const auto metadata = std::find_if(data.datasets.begin(),data.datasets.end(),[&](const SimResults::Dataset& value) {
      return value.id == entity.dataset_id;
    });
    result.dataset_name = "Dataset " + std::to_string(entity.dataset_id);
    if (metadata != data.datasets.end() && !metadata->name.empty()) {
      result.dataset_name = metadata->name;
    }
    result.status = "Unavailable in this frame";
    if (display.increment_index >= data.increments.size()) {
      return result;
    }
    const auto& frame = data.increments[display.increment_index];
    result.step = frame.step;
    result.increment = frame.increment;
    result.time = frame.time;
    bool havePosition = false, multiplePositions = false;
    for (const auto& part : result_mesh_parts(frame.dataset.get())) {
      if (part.first != entity.dataset_id) {
        continue;
      }
      std::string issue;
      const auto matches = entity_indices(part.second,entity,issue);
      if (matches.empty()) {
        continue;
      }
      result.available = true;
      result.status = "Selected (hidden)";
      if (result.visible) {
        result.status = "Selected";
      }
      if (entity.kind != EntityKind::Element) {
        for (auto index : matches) {
          std::array<double,3> original;
          part.second->GetPoint(static_cast<vtkIdType>(index),original.data());
          const auto moved = deformed_position(part.second,index,part.first,frame,display.deformation);
          if (!havePosition) {
            result.original_position = original;
            result.displayed_position = moved;
            havePosition = true;
          }
          else if (*result.original_position != original || *result.displayed_position != moved) {
            multiplePositions = true;
          }
        }
      }
      if (!display.coloring.field) {
        result.value_status = "Solid color; no field selected";
        continue;
      }
      auto selection = *display.coloring.field;
      result.component = selection.component;
      if (selection.magnitude) {
        result.component = "Magnitude";
      }
      if (selection.dataset_id && *selection.dataset_id != part.first) {
        result.value_status = "The color field is scoped to another dataset";
        continue;
      }
      selection.dataset_id = part.first;
      const auto* field = find_field(frame,selection);
      if (!field || field->association == SimResults::Association::Table ||
          (entity.kind != EntityKind::Element && field->association == SimResults::Association::Element)) {
        result.value_status = "Field/component association unavailable";
        continue;
      }
      result.field = field->name;
      double unitValue = 0.0;
      if (!SimResults::ResultsUnits::field_value(0.0, *field, selection.component, selection.magnitude, "postprocessor", unitValue, result.value_unit, issue)) {
        result.value_status = issue;
        continue;
      }
      std::set<std::size_t> samples(matches.begin(),matches.end());
      if (entity.kind == EntityKind::Element && field->association != SimResults::Association::Element) {
        samples.clear();
        auto ids = vtkSmartPointer<vtkIdList>::New();
        for (auto index : matches) {
          part.second->GetCellPoints(static_cast<vtkIdType>(index),ids);
          for (vtkIdType i = 0; i < ids->GetNumberOfIds(); ++i) {
            samples.insert(static_cast<std::size_t>(ids->GetId(i)));
          }
        }
        result.range_value = true;
      }
      for (auto index : samples) {
        double value;
        if (!color_sample(part.second,*field,selection,index,value,issue) || !std::isfinite(value)) {
          ++result.invalid_samples;
          continue;
        }
        ++result.valid_samples;
        if (!result.values) {
          result.values = std::array<double,2>{value,value};
        }
        else {
          (*result.values)[0] = std::min((*result.values)[0],value);
          (*result.values)[1] = std::max((*result.values)[1],value);
        }
      }
    }
    result.presentation_original_position = result.original_position;
    result.presentation_displayed_position = result.displayed_position;
    const SimUnits::QuantityUnit coordinate("length", data.source_units.coordinate_unit, SimUnits::Length);
    std::string unitError;
    double displayed = 0.0;
    SimResults::ResultsUnits::display_value(0.0, coordinate, "postprocessor", displayed, result.coordinate_unit, unitError);
    for (auto* position : {&result.presentation_original_position, &result.presentation_displayed_position}) {
      if (!*position) {
        continue;
      }
      for (double& value : **position) {
        if (!SimResults::ResultsUnits::display_value(value, coordinate, "postprocessor", displayed, result.coordinate_unit, unitError)) {
          position->reset();
          break;
        }
        value = displayed;
      }
    }
    const auto& time = frame.independent_variable;
    if (SimResults::ResultsUnits::display_value(result.time, time, "postprocessor", displayed, result.time_unit, unitError)) {
      result.presentation_time = displayed;
    }
    if (entity.kind == EntityKind::Node) {
      result.material = "Shared nodes have no single material assignment";
    } else if (metadata != data.datasets.end() && !metadata->material_id_array.empty()) {
      std::set<int> ids;
      for (const auto& part : result_mesh_parts(frame.dataset.get())) {
        if (part.first != entity.dataset_id) {
          continue;
        }
        auto* materials = part.second->GetCellData()->GetArray(metadata->material_id_array.c_str());
        if (!materials) {
          continue;
        }
        for (auto index : entity_indices(part.second, entity, unitError)) {
          if (index < static_cast<std::size_t>(materials->GetNumberOfTuples())) {
            const int id = static_cast<int>(materials->GetComponent(static_cast<vtkIdType>(index), 0));
            if (id > 0) {
              ids.insert(id);
            }
          }
        }
      }
      FrameworkMaterialsInterface materials(metadata->material_workspace);
      for (int id : ids) {
        if (!result.material.empty()) {
          result.material += ", ";
        }
        result.material += metadata->material_workspace + "/" + std::to_string(id);
        if (metadata->material_project == framework_materials().project_id()) {
          const auto* material = materials.find(id);
          if (material) {
            result.material += " (" + material->data.name + ")";
          } else {
            result.material += " (unresolved)";
          }
        } else {
          result.material += " (source project " + metadata->material_project + "; unresolved)";
        }
      }
    }
    if (multiplePositions) {
      result.original_position.reset();
      result.displayed_position.reset();
      result.presentation_original_position.reset();
      result.presentation_displayed_position.reset();
      result.status += "; multiple positions for this identity";
    }
    if (result.values) {
      result.range_value = result.range_value || (*result.values)[0] != (*result.values)[1];
      result.value_status = "Stored value";
      if (result.range_value) {
        result.value_status = "Valid sample range (no averaging)";
      }
    }
    else if (result.available && result.value_status.empty()) {
      result.value_status = "Unavailable (missing or invalid samples)";
    }
    return result;
  }

  std::string probe_text(const ProbeResult& result) {
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::setprecision(12) << "Source: " << result.source_name << "\nDataset: " << result.dataset_name
      << " (" << result.entity.dataset_id << ")\n";
    if (result.entity.kind == EntityKind::Node) {
      text << "Node: ";
    } else {
      text << "Element: ";
    }
    text << result.entity.entity_id;
    if (result.entity.integration_point) {
      text << "\nIntegration point: " << *result.entity.integration_point;
    }
    text << "\nStatus: " << result.status;
    const auto position = [&](const char* label,const std::optional<std::array<double,3>>& value) {
      text << '\n' << label;
      if (value) {
        text << (*value)[0] << ", " << (*value)[1] << ", " << (*value)[2];
      }
      else {
        if (result.entity.kind == EntityKind::Element) {
          text << "Not applicable";
        } else {
          text << "Unavailable";
        }
      }
    };
    position("Original position: ", result.presentation_original_position);
    position("Displayed position: ", result.presentation_displayed_position);
    text << " [" << result.coordinate_unit << "]";
    if (!result.material.empty()) {
      text << "\nMaterial: " << result.material;
    }
    text << "\nField: ";
    if (result.field.empty()) {
      text << "Unavailable";
    } else {
      text << result.field;
    }
    text << "\nComponent: " << result.component;
    text << "\nValue: ";
    if (result.values) {
      text << (*result.values)[0];
      if (result.range_value) {
        text << " .. " << (*result.values)[1];
      }
    }
    else {
      text << "Unavailable";
    }
    text << " [" << result.value_unit << "]";
    text << "\n" << result.value_status << "\nValid / invalid samples: " << result.valid_samples << " / " << result.invalid_samples;
    text << "\nStep / increment: ";
    if (result.step < 0) {
      text << "Unknown";
    } else {
      text << result.step;
    }
    text << " / ";
    if (result.increment < 0) {
      text << "Unknown";
    } else {
      text << result.increment;
    }
    text << "\nAnalysis parameter: ";
    if (result.presentation_time) {
      text << *result.presentation_time;
    } else {
      text << result.time;
    }
    text << " [" << result.time_unit << "]";
    return text.str();
  }
}
