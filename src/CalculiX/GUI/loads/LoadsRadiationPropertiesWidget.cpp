#include "LoadsRadiationPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsRadiationPropertiesWidget::LoadsRadiationPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsRadiationPropertiesWidget::setRadiation(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsRadiationSettings settings;
  std::string error;
  if (!core.get_radiation_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Radiation properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Radiation", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Sideset, "Sideset", QString::fromStdString(settings.sideset));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Temperature", QString::fromStdString(settings.temperature));
  addValue(components, "Emissivity", QString::fromStdString(settings.coefficient));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Radiation amplitude", QString::fromStdString(settings.secondaryAmplitude));
  addValue(settingsGroup, "Radiation Time Delay", QString::fromStdString(settings.secondaryTimeDelay));
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addValue(settingsGroup, "Cavity", QString::fromStdString(settings.cavity));
  addValue(settingsGroup, "Cavity Radiation", QString::fromStdString(settings.cavityRadiation));
  return true;
}

void LoadsRadiationPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsRadiationSettings settings;
  std::string error;
  if (!core.get_radiation_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify radiation %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setRadiation(id); });
}
