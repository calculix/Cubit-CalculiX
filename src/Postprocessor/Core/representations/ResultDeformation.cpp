#include "ResultDeformation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "ResultMesh.hpp"
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <algorithm>
#include <cmath>

namespace Postprocessor
{
  static bool displacement_scales(const SimResults::Field& field, const SimResults::Increment& frame, std::array<double, 3>& scales, std::string& error) {
    scales = {1.0, 1.0, 1.0};
    if (field.component_units.empty() && frame.coordinate_unit.empty()) {
      return true;
    }
    if (!field.vector_components || field.component_units.size() != field.components.size()) {
      error = "Displacement quantity metadata is incomplete.";
      return false;
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
      const auto component = (*field.vector_components)[axis];
      if (component >= field.component_units.size()) {
        error = "Displacement component metadata is invalid.";
        return false;
      }
      const auto& quantity = field.component_units[component];
      if (quantity.unit.empty() && frame.coordinate_unit.empty()) {
        continue;
      }
      if (quantity.dimension != SimUnits::Length || quantity.role != SimUnits::UnitRole::Linear || quantity.unit.empty() || frame.coordinate_unit.empty() || !framework_units().convert(1.0, quantity.unit, frame.coordinate_unit, scales[axis], error)) {
        error = "Displacement and coordinate units must both be known compatible lengths.";
        return false;
      }
    }
    return true;
  }
  bool same_deformation(const ResultDeformation& a, const ResultDeformation& b) {
    return a.enabled == b.enabled && a.field_mode == b.field_mode &&
      same_field(a.field,b.field) && a.scale == b.scale && a.overlay == b.overlay;
  }

  bool valid_deformation(const ResultDeformation& value, std::string& error) {
    error.clear();
    if (!std::isfinite(value.scale) || value.scale < 0 ||
        (value.field_mode != DeformationFieldMode::Automatic && value.field_mode != DeformationFieldMode::Explicit) ||
        (value.enabled && value.field_mode == DeformationFieldMode::Explicit && !value.field)) {
      error = "Deformation requires a finite nonnegative scale and Automatic or an explicit vector selection.";
      return false;
    }
    return true;
  }

  bool deformation_vector(const SimResults::Field& field) {
    if (!field.vector_components || (field.association != SimResults::Association::Node &&
        field.association != SimResults::Association::IntegrationPoint)) {
      return false;
    }
    const auto& xyz = *field.vector_components;
    return xyz[0] != xyz[1] && xyz[0] != xyz[2] && xyz[1] != xyz[2] &&
      std::all_of(xyz.begin(),xyz.end(),[&](std::size_t i) {
        return i < field.components.size();
      });
  }

  const SimResults::Field* resolve_deformation_field(const SimResults::Increment& frame,
    std::size_t dataset, const ResultDeformation& settings, std::string& warning) {
    warning.clear();
    if (settings.field_mode == DeformationFieldMode::Explicit) {
      if (settings.field && settings.field->dataset_id && *settings.field->dataset_id != dataset) {
        return nullptr;
      }
      if (settings.field) {
        auto selection = *settings.field;
        selection.dataset_id = dataset;
        const auto* field = find_field(frame,selection);
        if (field && deformation_vector(*field)) {
          return field;
        }
      }
      warning = "Selected displacement vector unavailable; showing original coordinates.";
      return nullptr;
    }
    const SimResults::Field* match = nullptr;
    for (const auto& field : frame.fields) {
      if (field.dataset_id != dataset || field.role != SimResults::FieldRole::Displacement || !deformation_vector(field)) {
        continue;
      }
      if (match) {
        warning = "Automatic displacement is ambiguous; showing original coordinates. Select an explicit field.";
        return nullptr;
      }
      match = &field;
    }
    if (!match) {
      warning = "No role-marked displacement vector available; showing original coordinates.";
    }
    return match;
  }

  bool deformation_available(const SimResults::Increment& frame,
    const ResultDeformation& settings, std::string& error) {
    error.clear();
    for (const auto& part : result_mesh_parts(frame.dataset.get())) {
      std::string warning;
      if (vtkPointSet::SafeDownCast(part.second) && resolve_deformation_field(frame,part.first,settings,warning)) {
        return true;
      }
    }
    error = "No mesh dataset has an available, unambiguous displacement vector for this selection.";
    return false;
  }

  std::array<double,3> deformed_position(vtkDataSet* mesh, std::size_t point, std::size_t dataset,
    const SimResults::Increment& frame, const ResultDeformation& settings) {
    std::array<double,3> original;
    mesh->GetPoint(static_cast<vtkIdType>(point),original.data());
    std::string warning;
    if (!settings.enabled || !valid_deformation(settings,warning)) {
      return original;
    }
    const auto* field = resolve_deformation_field(frame,dataset,settings,warning);
    if (!field) {
      return original;
    }
    auto* values = mesh->GetPointData()->GetArray(field->array_name.c_str());
    vtkDataArray* mask = nullptr;
    if (!field->validity_array.empty()) {
      mask = mesh->GetPointData()->GetArray(field->validity_array.c_str());
    }
    const auto count = mesh->GetNumberOfPoints();
    if (!values || values->GetNumberOfTuples() != count || values->GetNumberOfComponents() != static_cast<int>(field->components.size()) ||
        (!field->validity_array.empty() && (!mask || mask->GetNumberOfTuples() != count || mask->GetNumberOfComponents() != 1)) ||
        (mask && (!std::isfinite(mask->GetComponent(static_cast<vtkIdType>(point),0)) || mask->GetComponent(static_cast<vtkIdType>(point),0) <= 0))) {
      return original;
    }
    auto moved = original;
    std::array<double, 3> scales;
    if (!displacement_scales(*field, frame, scales, warning)) {
      return original;
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
      const double value = values->GetComponent(static_cast<vtkIdType>(point),static_cast<int>((*field->vector_components)[axis]));
      if (!std::isfinite(value)) {
        return original;
      }
      moved[axis] += settings.scale * value * scales[axis];
      if (!std::isfinite(moved[axis])) {
        return original;
      }
    }
    return moved;
  }

  bool deform_mesh(vtkDataSet* mesh, std::size_t dataset, const SimResults::Increment& frame,
    const ResultDeformation& settings, std::shared_ptr<vtkDataSet>& output,
    std::string& warning, std::string& error) {
    output.reset();
    warning.clear();
    if (!valid_deformation(settings,error)) {
      return false;
    }
    if (!settings.enabled) {
      return true;
    }
    const auto* field = resolve_deformation_field(frame,dataset,settings,warning);
    if (!field) {
      return true;
    }
    auto* pointsMesh = vtkPointSet::SafeDownCast(mesh);
    if (!pointsMesh) {
      warning = "Displacement field or compatible point mesh unavailable; showing original coordinates.";
      return true;
    }
    auto* values = mesh->GetPointData()->GetArray(field->array_name.c_str());
    vtkDataArray* mask = nullptr;
    if (!field->validity_array.empty()) {
      mask = mesh->GetPointData()->GetArray(field->validity_array.c_str());
    }
    const auto count = mesh->GetNumberOfPoints();
    if (!values || values->GetNumberOfTuples() != count ||
        values->GetNumberOfComponents() != static_cast<int>(field->components.size()) ||
        (!field->validity_array.empty() && (!mask || mask->GetNumberOfTuples() != count || mask->GetNumberOfComponents() != 1))) {
      warning = "Displacement array or validity mask unavailable; showing original coordinates.";
      return true;
    }
    auto points = vtkSmartPointer<vtkPoints>::New();
    std::array<double, 3> scales;
    if (!displacement_scales(*field, frame, scales, warning)) {
      return true;
    }
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(count);
    vtkIdType invalid = 0;
    for (vtkIdType i = 0; i < count; ++i) {
      double original[3], moved[3];
      mesh->GetPoint(i,original);
      bool valid = !mask || (std::isfinite(mask->GetComponent(i,0)) && mask->GetComponent(i,0) > 0);
      for (std::size_t axis = 0; axis < 3; ++axis) {
        const double u = values->GetComponent(i,static_cast<int>((*field->vector_components)[axis]));
        moved[axis] = original[axis] + settings.scale * u * scales[axis];
        valid = valid && std::isfinite(u) && std::isfinite(moved[axis]);
      }
      if (valid) {
        points->SetPoint(i, moved);
      } else {
        points->SetPoint(i, original);
      }
      if (!valid) {
        ++invalid;
      }
    }
    std::shared_ptr<vtkDataSet> candidate(pointsMesh->NewInstance(),[](vtkDataSet* value) {
      value->Delete();
    });
    candidate->ShallowCopy(mesh);
    vtkPointSet::SafeDownCast(candidate.get())->SetPoints(points);
    output.swap(candidate);
    if (invalid != 0) {
      warning = std::to_string(invalid)+" invalid displacement samples retain original coordinates.";
    }
    return true;
  }
}
