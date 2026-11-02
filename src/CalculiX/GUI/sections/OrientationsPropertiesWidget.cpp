#include "OrientationsPropertiesWidget.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "CubitMessage.hpp"

OrientationsPropertiesWidget::OrientationsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool OrientationsPropertiesWidget::setOrientation(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXSectionsInterface core;
  OrientationsSettings settings;
  std::string error;
  if (!core.get_orientation_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Orientation properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Orientation", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "System Type", QString::fromStdString(settings.system));
  addValue(settingsGroup, "a coord", QString::fromStdString(settings.a));
  addValue(settingsGroup, "b coord", QString::fromStdString(settings.b));
  addValue(settingsGroup, "local axis", QString::fromStdString(settings.axis));
  addValue(settingsGroup, "angle", QString::fromStdString(settings.angle));
  return true;
}

void OrientationsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXSectionsInterface core;
  OrientationsSettings settings;
  std::string error;
  if (!core.get_orientation_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify orientation %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setOrientation(id); });
}
