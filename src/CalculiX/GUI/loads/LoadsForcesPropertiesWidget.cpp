#include "LoadsForcesPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsForcesPropertiesWidget::LoadsForcesPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsForcesPropertiesWidget::setForce(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsForcesSettings settings;
  std::string error;
  if (!core.get_force_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Force properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Force", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.nodeset));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Force X", QString::fromStdString(settings.component1));
  addValue(components, "Force Y", QString::fromStdString(settings.component2));
  addValue(components, "Force Z", QString::fromStdString(settings.component3));
  addValue(components, "Moment X", QString::fromStdString(settings.component4));
  addValue(components, "Moment Y", QString::fromStdString(settings.component5));
  addValue(components, "Moment Z", QString::fromStdString(settings.component6));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  return true;
}

void LoadsForcesPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsForcesSettings settings;
  std::string error;
  if (!core.get_force_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify force %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setForce(id); });
}
