#include "CalculiXJobRunRecord.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
#include <QFile>
#include <QUuid>
#include <utility>

CalculiXJobRunRecord::CalculiXJobRunRecord() = default;

bool CalculiXJobRunRecord::read_input_units(const std::string& filename, std::string& error)
{
  error.clear();
  QFile file(QString::fromStdString(filename));

  if (!file.open(QIODevice::ReadOnly))
  {
    error = "Cannot read input metadata: " + file.errorString().toStdString();
    return false;
  }

  constexpr qint64 maximumBytes = 128LL * 1024 * 1024;

  if (file.size() > maximumBytes)
  {
    error = "Input metadata file exceeds the supported size.";
    return false;
  }

  CalculiXJobRunRecord candidate;
  candidate.input.rootFile = filename;
  candidate.input.files.emplace(filename, file.readAll());

  if (file.error() != QFileDevice::NoError)
  {
    error = "Cannot read input metadata: " + file.errorString().toStdString();
    return false;
  }

  if (!candidate.read_input_units(error))
  {
    return false;
  }

  materialProjectId = std::move(candidate.materialProjectId);
  materialIds = std::move(candidate.materialIds);
  unitSystem = std::move(candidate.unitSystem);
  step_axis_quantities = std::move(candidate.step_axis_quantities);
  geometryLengthUnit = std::move(candidate.geometryLengthUnit);
  return true;
}

bool CalculiXJobRunRecord::read_input_units(std::string& error)
{
  error.clear();
  const auto file = input.files.find(input.rootFile);

  // Metadata-only topology records have no retained input. Persistence restores
  // their provenance directly, so validate it below without clearing it.
  if (file != input.files.end())
  {
    materialProjectId.clear();
    materialIds.clear();
    unitSystem.clear();
    step_axis_quantities.clear();
    geometryLengthUnit.clear();

    for (const QByteArray& raw : file->second.split('\n'))
    {
      const QByteArray line = raw.trimmed();
      if (line.startsWith("** CUBIT_SIM_STEP_AXIS="))
      {
        const auto value = line.mid(QByteArray("** CUBIT_SIM_STEP_AXIS=").size());
        const int separator = value.indexOf(';');
        bool valid = false;
        const int step = value.left(separator).toInt(&valid);
        const std::string quantity = value.mid(separator + 1).trimmed().toStdString();
        if (!valid || step <= 0 || separator <= 0 ||
            (quantity != "time" && quantity != "frequency") ||
            !step_axis_quantities.emplace(step, quantity).second)
        {
          error = "Invalid or duplicate solver-step axis provenance.";
          return false;
        }
      }
      if (line.startsWith("** CUBIT_SIM_PROJECT="))
      {
        materialProjectId = line.mid(QByteArray("** CUBIT_SIM_PROJECT=").size()).trimmed().toStdString();
      }
      if (line.startsWith("** CUBIT_SIM_MATERIAL_ID="))
      {
        const QByteArray value = line.mid(QByteArray("** CUBIT_SIM_MATERIAL_ID=").size());
        const int separator = value.indexOf(';');
        bool valid = false;
        const int id = value.left(separator).toInt(&valid);
        const std::string name = value.mid(separator + 1).trimmed().toStdString();
        if (separator <= 0 || !valid || id <= 0 || name.empty() || !materialIds.emplace(name, id).second)
        {
          error = "The input deck contains invalid or duplicate material provenance.";
          materialIds.clear();
          return false;
        }
      }
      if (line.startsWith("** CUBIT_SIM_UNIT_SYSTEM="))
      {
        unitSystem = line.mid(QByteArray("** CUBIT_SIM_UNIT_SYSTEM=").size()).trimmed().toStdString();
      }
      if (line.startsWith("** CUBIT_SIM_GEOMETRY_UNIT="))
      {
        geometryLengthUnit = line.mid(QByteArray("** CUBIT_SIM_GEOMETRY_UNIT=").size()).trimmed().toStdString();
      }
    }
  }
  if (!unitSystem.empty())
  {
    const auto systems = framework_units().system_ids();
    if (std::find(systems.begin(), systems.end(), unitSystem) == systems.end())
    {
      error = "The input deck declares an unsupported unit system: " + unitSystem;
      unitSystem.clear();
      geometryLengthUnit.clear();
      return false;
    }
  }
  if (!geometryLengthUnit.empty())
  {
    SimUnits::UnitDefinition geometry;
    if (!framework_units().unit(geometryLengthUnit, geometry, error) || geometry.dimension != SimUnits::Length || geometry.offset != 0.0)
    {
      error = "The input deck declares an invalid geometry length unit.";
      return false;
    }
  }
  for (const auto& axis : step_axis_quantities)
  {
    if (axis.first <= 0 || (axis.second != "time" && axis.second != "frequency"))
    {
      error = "Invalid saved solver-step axis provenance.";
      return false;
    }
  }
  for (const auto& material : materialIds)
  {
    if (material.first.empty() || material.second <= 0)
    {
      error = "Invalid saved material provenance.";
      return false;
    }
  }
  if ((!materialProjectId.empty() && QUuid(QString::fromStdString(materialProjectId)).isNull()) || (!materialIds.empty() && materialProjectId.empty()))
  {
    error = "The input deck contains material IDs without a valid project identity.";
    return false;
  }
  return true;
}
