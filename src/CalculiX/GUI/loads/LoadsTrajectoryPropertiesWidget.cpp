#include "LoadsTrajectoryPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsTrajectoryPropertiesWidget::LoadsTrajectoryPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsTrajectoryPropertiesWidget::setTrajectory(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsTrajectorySettings settings;
  std::string error;
  if (!core.get_trajectory_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Trajectory properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Trajectory", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Load Type", QString::fromStdString(settings.loadType));
  addValue(settingsGroup, "Time Begin", QString::fromStdString(settings.timeBegin));
  addValue(settingsGroup, "Time End", QString::fromStdString(settings.timeEnd));
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Curve, "Curve", QString::fromStdString(settings.curve));
  addReference(assignment, PropertyReferenceKind::Vertex, "Vertex", QString::fromStdString(settings.vertex));
  addValue(assignment, "Surface IDs", QString::fromStdString(settings.surfaces));
  addValue(assignment, "Surface names", PropertyReferenceResolver::names(PropertyReferenceKind::Surface, QString::fromStdString(settings.surfaces)));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Direction", QString::fromStdString(settings.direction));
  if (settings.type == 1)
  {
    addTable("trajectory_samples", "Trajectory samples", {"Radius", "Magnitude"}, settings.samples);
  }
  else
  {
    addTable("trajectory_samples", "Trajectory samples", {"Radius", "Depth", "Magnitude"}, settings.samples);
  }
  return true;
}

void LoadsTrajectoryPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsTrajectorySettings settings;
  std::string error;
  if (!core.get_trajectory_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  QString subtype;
  switch (settings.type)
  {
    case 1: subtype = "heatflux"; break;
    case 2: subtype = "bodyheatfluxsphere"; break;
    default: return;
  }
  const QString command = QString("ccx modify trajectory %1 %2 name %3").arg(request.entityId).arg(subtype).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setTrajectory(id); });
}
