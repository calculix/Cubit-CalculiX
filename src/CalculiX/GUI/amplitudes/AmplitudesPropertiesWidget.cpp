#include "AmplitudesPropertiesWidget.hpp"
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "CubitMessage.hpp"

AmplitudesPropertiesWidget::AmplitudesPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool AmplitudesPropertiesWidget::setAmplitude(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXAmplitudesInterface core;
  AmplitudesSettings settings;
  std::string error;
  if (!core.get_amplitude_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Amplitude properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Amplitude", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Shift x", QString::fromStdString(settings.shiftX));
  addValue(settingsGroup, "Shift y", QString::fromStdString(settings.shiftY));
  addValue(settingsGroup, "Time=TotalTime", QString::fromStdString(settings.totalTime));
  if (!settings.points.empty())
  {
    addTable("points", "Points", {"Time", "Amplitude"}, settings.points);
  }
  return true;
}

void AmplitudesPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXAmplitudesInterface core;
  AmplitudesSettings settings;
  std::string error;
  if (!core.get_amplitude_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify amplitude %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setAmplitude(id); });
}
