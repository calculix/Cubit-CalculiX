#include "BCsDisplacementsPropertiesWidget.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "CubitMessage.hpp"

BCsDisplacementsPropertiesWidget::BCsDisplacementsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool BCsDisplacementsPropertiesWidget::setDisplacement(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXBCsInterface core;
  BCsDisplacementsSettings settings;
  std::string error;
  if (!core.get_displacement_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Displacement properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Displacement", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.nodeset));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Displacement X", QString::fromStdString(settings.component1));
  addValue(components, "Displacement Y", QString::fromStdString(settings.component2));
  addValue(components, "Displacement Z", QString::fromStdString(settings.component3));
  addValue(components, "Rotation X", QString::fromStdString(settings.component4));
  addValue(components, "Rotation Y", QString::fromStdString(settings.component5));
  addValue(components, "Rotation Z", QString::fromStdString(settings.component6));
  return true;
}

void BCsDisplacementsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXBCsInterface core;
  BCsDisplacementsSettings settings;
  std::string error;
  if (!core.get_displacement_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify displacement %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setDisplacement(id); });
}
