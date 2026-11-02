#include "CustomLinesPropertiesWidget.hpp"
#include "Core/export/CalculiXExportInterface.hpp"
#include "CubitMessage.hpp"

CustomLinesPropertiesWidget::CustomLinesPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool CustomLinesPropertiesWidget::setCustomLine(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXExportInterface core;
  CustomLinesSettings settings;
  std::string error;
  if (!core.get_customline_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load CustomLine properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Custom Line", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Position", QString::fromStdString(settings.position));
  addValue(settingsGroup, "Item", QString::fromStdString(settings.keyword));
  addValue(settingsGroup, "Item ID", QString::fromStdString(settings.keywordId));
  addValue(settingsGroup, "Custom Line", QString::fromStdString(settings.text));
  return true;
}

void CustomLinesPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXExportInterface core;
  CustomLinesSettings settings;
  std::string error;
  if (!core.get_customline_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify customline %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setCustomLine(id); });
}
