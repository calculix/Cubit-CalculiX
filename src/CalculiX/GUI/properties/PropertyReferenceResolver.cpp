#include "PropertyReferenceResolver.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include <QCoreApplication>
#include <QStringList>
#include <string>

namespace
{
QString resolved(bool exists, const std::string& name)
{
  if (!exists)
  {
    return QCoreApplication::translate("PropertyReferenceResolver", "Not found");
  }
  return name.empty() ? QCoreApplication::translate("PropertyReferenceResolver", "Unnamed") : QString::fromStdString(name);
}
}

QString PropertyReferenceResolver::name(PropertyReferenceKind kind, const QString& value)
{
  const QString text = value.trimmed();
  if (text.isEmpty() || text == "0" || text == "-1")
  {
    return QCoreApplication::translate("PropertyReferenceResolver", "Not assigned");
  }
  bool valid = false;
  const int id = text.toInt(&valid);
  if (!valid || id < 1)
  {
    return resolved(false, {});
  }
  std::string error;
  switch (kind)
  {
    case PropertyReferenceKind::Block:
    {
      CalculiXMeshInterface core;
      std::string entityName;
      const bool exists = core.get_assignment_entity_name("block", id, entityName);
      return resolved(exists, entityName);
    }
    case PropertyReferenceKind::Nodeset:
    {
      CalculiXMeshInterface core;
      std::string entityName;
      const bool exists = core.get_assignment_entity_name("nodeset", id, entityName);
      return resolved(exists, entityName);
    }
    case PropertyReferenceKind::Sideset:
    {
      CalculiXMeshInterface core;
      std::string entityName;
      const bool exists = core.get_assignment_entity_name("sideset", id, entityName);
      return resolved(exists, entityName);
    }
    case PropertyReferenceKind::Material:
    {
      CalculiXMaterialsInterface core;
      if (!core.check_material_exists(id))
      {
        return resolved(false, {});
      }
      return resolved(true, core.get_material_name(id));
    }
    case PropertyReferenceKind::Orientation:
    {
      CalculiXSectionsInterface core;
      if (!core.check_orientation_exists(id))
      {
        return resolved(false, {});
      }
      return resolved(true, core.get_orientation_name(id));
    }
    case PropertyReferenceKind::Amplitude:
    {
      CalculiXAmplitudesInterface core;
      AmplitudesSettings settings;
      const bool exists = core.get_amplitude_settings(id, settings, error);
      return resolved(exists, settings.name);
    }
    case PropertyReferenceKind::Step:
    {
      CalculiXStepsInterface core;
      StepsSettings settings;
      const bool exists = core.get_step_settings(id, settings, error);
      return resolved(exists, settings.name);
    }
    case PropertyReferenceKind::Job:
    {
      CalculiXJobsInterface core;
      JobsSettings settings;
      const bool exists = core.get_job_settings(id, settings, error);
      return resolved(exists, settings.name);
    }
    case PropertyReferenceKind::SurfaceInteraction:
    {
      CalculiXInteractionsInterface core;
      SurfaceInteractionsSettings settings;
      const bool exists = core.get_surfaceinteraction_settings(id, settings, error);
      return resolved(exists, settings.name);
    }
    case PropertyReferenceKind::ContactPair:
    {
      CalculiXInteractionsInterface core;
      ContactPairsSettings settings;
      const bool exists = core.get_contactpair_settings(id, settings, error);
      return resolved(exists, {});
    }
    case PropertyReferenceKind::Displacement:
    {
      CalculiXBCsInterface core;
      BCsDisplacementsSettings settings;
      const bool exists = core.get_displacement_settings(id, settings, error);
      return resolved(exists, settings.name);
    }
    case PropertyReferenceKind::Temperature:
    {
      CalculiXBCsInterface core;
      BCsTemperaturesSettings settings;
      const bool exists = core.get_temperature_settings(id, settings, error);
      return resolved(exists, settings.name);
    }
    case PropertyReferenceKind::Vertex:
    {
      CalculiXMeshInterface core;
      std::string entityName;
      const bool exists = core.get_assignment_entity_name("vertex", id, entityName);
      return resolved(exists, entityName);
    }
    case PropertyReferenceKind::Curve:
    {
      CalculiXMeshInterface core;
      std::string entityName;
      const bool exists = core.get_assignment_entity_name("curve", id, entityName);
      return resolved(exists, entityName);
    }
    case PropertyReferenceKind::Surface:
    {
      CalculiXMeshInterface core;
      std::string entityName;
      const bool exists = core.get_assignment_entity_name("surface", id, entityName);
      return resolved(exists, entityName);
    }
    case PropertyReferenceKind::Topology:
    {
      CalculiXTopologyInterface core;
      TopologyOptions settings;
      const bool exists = core.get_topology_settings(id, settings);
      return resolved(exists, settings.name);
    }
  }
  return resolved(false, {});
}

QString PropertyReferenceResolver::names(PropertyReferenceKind kind, const QString& ids)
{
  if (ids.trimmed().isEmpty())
  {
    return name(kind, {});
  }
  CalculiXApplicationInterface core;
  const auto parsed = core.parser("", ids.toStdString());
  if (parsed.empty())
  {
    return resolved(false, {});
  }
  QStringList values;
  for (int id : parsed)
  {
    const QString text = QString::number(id);
    values.append(text + ": " + name(kind, text));
  }
  return values.join("; ");
}
