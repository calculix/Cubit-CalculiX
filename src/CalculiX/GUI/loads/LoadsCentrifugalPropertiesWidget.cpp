#include "LoadsCentrifugalPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsCentrifugalPropertiesWidget::LoadsCentrifugalPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsCentrifugalPropertiesWidget::setCentrifugal(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsCentrifugalSettings settings;
  std::string error;
  if (!core.get_centrifugal_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Centrifugal properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Centrifugal", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Magnitude", QString::fromStdString(settings.magnitude));
  addValue(components, "Coordinate", QString::fromStdString(settings.coordinate));
  addValue(components, "Direction", QString::fromStdString(settings.direction));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.block));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  return true;
}

void LoadsCentrifugalPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsCentrifugalSettings settings;
  std::string error;
  if (!core.get_centrifugal_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify centrifugal %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setCentrifugal(id); });
}
