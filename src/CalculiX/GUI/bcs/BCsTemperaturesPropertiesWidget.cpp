#include "BCsTemperaturesPropertiesWidget.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "CubitMessage.hpp"

BCsTemperaturesPropertiesWidget::BCsTemperaturesPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool BCsTemperaturesPropertiesWidget::setTemperature(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXBCsInterface core;
  BCsTemperaturesSettings settings;
  std::string error;
  if (!core.get_temperature_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Temperature properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Temperature", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.nodeset));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Magnitude", QString::fromStdString(settings.magnitude));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  addValue(settingsGroup, "Keyword Type", QString::fromStdString(settings.keyword));
  return true;
}

void BCsTemperaturesPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXBCsInterface core;
  BCsTemperaturesSettings settings;
  std::string error;
  if (!core.get_temperature_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify temperature %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setTemperature(id); });
}
