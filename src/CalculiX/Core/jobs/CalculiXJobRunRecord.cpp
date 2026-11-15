#include "CalculiXJobRunRecord.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
#include <QUuid>

CalculiXJobRunRecord::CalculiXJobRunRecord() = default;

bool CalculiXJobRunRecord::read_input_units(std::string& error) {
  error.clear();
  materialProjectId.clear();
  materialIds.clear();
  unitSystem.clear();
  geometryLengthUnit.clear();
  const auto file = input.files.find(input.rootFile);
  if (file == input.files.end()) {
    return true;
  }
  for (const QByteArray& raw : file->second.split('\n')) {
    const QByteArray line = raw.trimmed();
    if (line.startsWith("** CUBIT_SIM_PROJECT=")) {
      materialProjectId = line.mid(QByteArray("** CUBIT_SIM_PROJECT=").size()).trimmed().toStdString();
    }
    if (line.startsWith("** CUBIT_SIM_MATERIAL_ID=")) {
      const QByteArray value = line.mid(QByteArray("** CUBIT_SIM_MATERIAL_ID=").size());
      const int separator = value.indexOf(';');
      bool valid = false;
      const int id = value.left(separator).toInt(&valid);
      const std::string name = value.mid(separator + 1).trimmed().toStdString();
      if (separator <= 0 || !valid || id <= 0 || name.empty() || !materialIds.emplace(name, id).second) {
        error = "The input deck contains invalid or duplicate material provenance.";
        materialIds.clear();
        return false;
      }
    }
    if (line.startsWith("** CUBIT_SIM_UNIT_SYSTEM=")) {
      unitSystem = line.mid(QByteArray("** CUBIT_SIM_UNIT_SYSTEM=").size()).trimmed().toStdString();
    }
    if (line.startsWith("** CUBIT_SIM_GEOMETRY_UNIT=")) {
      geometryLengthUnit = line.mid(QByteArray("** CUBIT_SIM_GEOMETRY_UNIT=").size()).trimmed().toStdString();
    }
  }
  if (!unitSystem.empty()) {
    const auto systems = framework_units().system_ids();
    if (std::find(systems.begin(), systems.end(), unitSystem) == systems.end()) {
      error = "The input deck declares an unsupported unit system: " + unitSystem;
      unitSystem.clear();
      geometryLengthUnit.clear();
      return false;
    }
  }
  if (!geometryLengthUnit.empty()) {
    SimUnits::UnitDefinition geometry;
    if (!framework_units().unit(geometryLengthUnit, geometry, error) || geometry.dimension != SimUnits::Length || geometry.offset != 0.0) {
      error = "The input deck declares an invalid geometry length unit.";
      return false;
    }
  }
  if ((!materialProjectId.empty() && QUuid(QString::fromStdString(materialProjectId)).isNull()) || (!materialIds.empty() && materialProjectId.empty())) {
    error = "The input deck contains material IDs without a valid project identity.";
    return false;
  }
  return true;
}
