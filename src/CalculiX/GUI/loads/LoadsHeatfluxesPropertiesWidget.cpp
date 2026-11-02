#include "LoadsHeatfluxesPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsHeatfluxesPropertiesWidget::LoadsHeatfluxesPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsHeatfluxesPropertiesWidget::setHeatflux(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsHeatfluxesSettings settings;
  std::string error;
  if (!core.get_heatflux_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Heatflux properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Heatflux", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Sideset, "Sideset", QString::fromStdString(settings.sideset));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Magnitude", QString::fromStdString(settings.magnitude));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  return true;
}

void LoadsHeatfluxesPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsHeatfluxesSettings settings;
  std::string error;
  if (!core.get_heatflux_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify heatflux %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setHeatflux(id); });
}
