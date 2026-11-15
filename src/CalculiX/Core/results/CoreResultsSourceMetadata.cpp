#include "CoreResultsSourceMetadata.hpp"
#include "Core/jobs/CalculiXJobRunRecord.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <cmath>
#include <limits>
CoreResultsSourceMetadata::CoreResultsSourceMetadata() = default;
CoreResultsSourceMetadata::CoreResultsSourceMetadata(const CalculiXJobRunRecord& record) : unitSystem(record.unitSystem), launchId(record.launchId), materialProjectId(record.materialProjectId), materialIds(record.materialIds) {
}
QJsonObject CoreResultsSourceMetadata::to_json() const {
  QJsonObject value;
  value.insert("version", 1);
  value.insert("system", QString::fromUtf8(unitSystem.c_str()));
  value.insert("launch", QString::fromUtf8(launchId.c_str()));
  value.insert("project", QString::fromUtf8(materialProjectId.c_str()));
  QJsonObject materials;
  for (const auto& entry : materialIds) {
    materials.insert(QString::fromUtf8(entry.first.c_str()), entry.second);
  }
  value.insert("materials", materials);
  return value;
}
bool CoreResultsSourceMetadata::from_json(const QJsonObject& value, CoreResultsSourceMetadata& output, std::string& error) {
  if (value.value("version").toInt(-1) != 1 || !value.value("system").isString() || !value.value("launch").isString() || !value.value("project").isString() || !value.value("materials").isObject()) {
    error = "Invalid saved result unit provenance.";
    return false;
  }
  CoreResultsSourceMetadata candidate;
  candidate.unitSystem = value.value("system").toString().toUtf8().toStdString();
  candidate.launchId = value.value("launch").toString().toUtf8().toStdString();
  candidate.materialProjectId = value.value("project").toString().toUtf8().toStdString();
  SimUnits::SourceUnitContext context;
  context.system = candidate.unitSystem;
  if (!framework_units().validate_source(context, error)) {
    return false;
  }
  const auto materials = value.value("materials").toObject();
  for (auto entry = materials.begin(); entry != materials.end(); ++entry) {
    const double id = entry.value().toDouble(-1);
    if (entry.key().isEmpty() || id < 1 || id > std::numeric_limits<int>::max() || std::floor(id) != id || candidate.materialProjectId.empty()) {
      error = "Invalid scoped material reference in saved results.";
      return false;
    }
    candidate.materialIds.emplace(entry.key().toUtf8().toStdString(), static_cast<int>(id));
  }
  output = std::move(candidate);
  return true;
}
