#include "CoreResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Results/VtkResultsMetadataCodec.hpp"
#include <vtkTable.h>
#include <vtkFieldData.h>
#include <vtkStringArray.h>
#include <algorithm>
#include <cctype>
#include <map>
#include <set>

namespace {
std::string field_quantity(const SimResults::Field& field) {
  if (field.role == SimResults::FieldRole::Displacement) {
    return "length";
  }
  std::string name = field.name;
  name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char value) {
    return std::isspace(value) != 0;
  }), name.end());
  std::transform(name.begin(), name.end(), name.begin(), [](unsigned char value) {
    return static_cast<char>(std::tolower(value));
  });
  const std::map<std::string, std::string> names = {{"disp", "length"}, {"displacements", "length"}, {"stress", "stress"}, {"stresses", "stress"}, {"tostrain", "strain"}, {"mestrain", "strain"}, {"strains", "strain"}, {"forc", "force"}, {"forces", "force"}, {"ndtemp", "temperature"}, {"temperatures", "temperature"}, {"velo", "velocity"}, {"velocities", "velocity"}, {"accel", "acceleration"}, {"accelerations", "acceleration"}};
  const auto found = names.find(name);
  if (found != names.end()) {
    return found->second;
  }
  return {};
}

bool resolve(const std::string& id, const SimUnits::SourceUnitContext& source, SimUnits::QuantityUnit& output, std::string& error) {
  if (id.empty()) {
    output = SimUnits::QuantityUnit();
    return true;
  }
  if (!source.system.empty()) {
    return framework_units().system_unit(source.system, id, output, error);
  }
  if (!framework_units().quantity(id, output, error)) {
    return false;
  }
  output.unit.clear();
  return true;
}

std::string table_text(vtkTable& table, const char* key) {
  auto* value = vtkStringArray::SafeDownCast(table.GetFieldData()->GetAbstractArray(key));
  if (value && value->GetNumberOfValues() > 0) {
    return value->GetValue(0);
  }
  return {};
}

std::string table_quantity(const std::string& field, const std::string& name, const std::string& column) {
  const std::set<std::string> identifiers = {"RecordIndex", "Valid", "NodeId", "ReportedNodeId", "SlaveElementId", "SlaveFace", "ElementId", "Face", "IntegrationPoint", "Count", "Mode", "BucklingFactor"};
  if (identifiers.count(column) != 0) {
    return "dimensionless";
  }
  if (column == "Time") {
    return "time";
  }
  if (field == "FREQUENCY") {
    if (column == "Eigenvalue") {
      return "frequency_squared";
    }
    if (column == "Frequency") {
      return "frequency";
    }
    if (column == "AngularFrequencyReal" || column == "AngularFrequencyImaginary") {
      return "angular_velocity";
    }
  }
  if (column == "X" || column == "Y" || column == "Z" || column == "CenterX" || column == "CenterY" || column == "CenterZ") {
    return "length";
  }
  if (column == "Fx" || column == "Fy" || column == "Fz" || column == "NormalForce" || column == "ShearForce") {
    return "force";
  }
  if (column == "MxOrigin" || column == "MyOrigin" || column == "MzOrigin" || column == "MxCenter" || column == "MyCenter" || column == "MzCenter" || column == "Torque" || column == "BendingMoment") {
    return "moment";
  }
  if (column == "NormalX" || column == "NormalY" || column == "NormalZ") {
    return "dimensionless";
  }
  if (column == "Area") {
    return "area";
  }
  if (column == "Energy" || column == "CELS") {
    return "energy";
  }
  if (column == "Volume") {
    return "volume";
  }
  if (column == "Mass") {
    return "mass";
  }
  if (column == "Ixx" || column == "Iyy" || column == "Izz" || column == "Ixy" || column == "Ixz" || column == "Iyz") {
    return "mass_moment";
  }
  if (column == "Heat" || column == "Heating") {
    return "power";
  }
  if (field == "FLUX" && column == "Flux") {
    if (name == "total surface flux") {
      return "power";
    }
    return "heat_flux";
  }
  if (field == "CSTR" || field == "DRAG") {
    return "stress";
  }
  if (field == "CDIS") {
    return "length";
  }
  if (column == "vx" || column == "vy" || column == "vz") {
    if (field == "U") {
      return "length";
    }
    if (field == "VF") {
      return "velocity";
    }
  }
  if (field == "RF" && (column == "fx" || column == "fy" || column == "fz")) {
    return "force";
  }
  return {};
}

bool tables(const std::vector<std::shared_ptr<vtkTable>>& tables, std::vector<SimResults::ResultsTableMetadata>& metadata, const SimUnits::SourceUnitContext& source, std::string& error) {
  metadata.resize(tables.size());
  for (std::size_t i = 0; i < tables.size(); ++i) {
    if (!tables[i]) {
      continue;
    }
    metadata[i].name = table_text(*tables[i], "TableName");
    const auto field = table_text(*tables[i], "Field");
    metadata[i].columns.resize(static_cast<std::size_t>(tables[i]->GetNumberOfColumns()));
    for (vtkIdType column = 0; column < tables[i]->GetNumberOfColumns(); ++column) {
      const auto* rawName = tables[i]->GetColumnName(column);
      if (!rawName) {
        continue;
      }
      const std::string columnName(rawName);
      const auto quantity = table_quantity(field, metadata[i].name, columnName);
      if (quantity == "dimensionless") {
        metadata[i].columns[static_cast<std::size_t>(column)] = SimUnits::QuantityUnit("dimensionless", "1", SimUnits::Dimensionless);
        continue;
      }
      if (!resolve(quantity, source, metadata[i].columns[static_cast<std::size_t>(column)], error)) {
        return false;
      }
    }
  }
  return true;
}
}

bool CoreResultsUnits::annotate(SimResults::VtkResultsData& result, std::string& error) {
  if (!framework_units().validate_source(result.source_units, error)) {
    return false;
  }
  for (auto& frame : result.increments) {
    frame.coordinate_unit = result.source_units.coordinate_unit;
    if (frame.mode < 0 && !resolve("time", result.source_units, frame.independent_variable, error)) {
      return false;
    }
    for (auto& field : frame.fields) {
      if (field.categorical) {
        field.component_units.assign(field.components.size(), SimUnits::QuantityUnit("", "1", SimUnits::Dimensionless));
        continue;
      }
      SimUnits::QuantityUnit quantity;
      if (!resolve(field_quantity(field), result.source_units, quantity, error)) {
        return false;
      }
      field.component_units.assign(field.components.size(), quantity);
    }
    if (!tables(frame.tables, frame.table_metadata, result.source_units, error)) {
      return false;
    }
  }
  if (!tables(result.tables, result.table_metadata, result.source_units, error)) {
    return false;
  }
  return SimResults::VtkResultsMetadataCodec::write(result, error);
}
