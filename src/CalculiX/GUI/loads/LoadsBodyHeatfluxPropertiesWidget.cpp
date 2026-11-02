#include "LoadsBodyHeatfluxPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsBodyHeatfluxPropertiesWidget::LoadsBodyHeatfluxPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsBodyHeatfluxPropertiesWidget::setBodyHeatflux(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsBodyHeatfluxSettings settings;
  std::string error;
  if (!core.get_bodyheatflux_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load BodyHeatflux properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Body Heatflux", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Magnitude", QString::fromStdString(settings.magnitude));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addValue(assignment, "Type", QString::fromStdString(settings.entityType));
  addValue(assignment, "IDs", QString::fromStdString(settings.ids));
  if (settings.entityType == "Block")
  {
    addValue(assignment, "Block names", PropertyReferenceResolver::names(PropertyReferenceKind::Block, QString::fromStdString(settings.ids)));
  }
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  return true;
}

void LoadsBodyHeatfluxPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsBodyHeatfluxSettings settings;
  std::string error;
  if (!core.get_bodyheatflux_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify bodyheatflux %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setBodyHeatflux(id); });
}
