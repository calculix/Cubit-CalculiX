#include "SurfaceInteractionsPropertiesWidget.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "CubitMessage.hpp"

SurfaceInteractionsPropertiesWidget::SurfaceInteractionsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool SurfaceInteractionsPropertiesWidget::setSurfaceInteraction(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXInteractionsInterface core;
  SurfaceInteractionsSettings settings;
  std::string error;
  if (!core.get_surfaceinteraction_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load SurfaceInteraction properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Surface Interaction", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Surface  Interaction ID", QString::fromStdString(settings.behavior));
  addValue(settingsGroup, "c0", QString::fromStdString(settings.c0));
  addValue(settingsGroup, "p0", QString::fromStdString(settings.p0));
  addValue(settingsGroup, "slope K", QString::fromStdString(settings.slope));
  addValue(settingsGroup, "sigmaINF", QString::fromStdString(settings.sigma));
  addValue(settingsGroup, "c0", QString::fromStdString(settings.linearC0));
  addValue(settingsGroup, "slope K", QString::fromStdString(settings.tiedSlope));
  addValue(settingsGroup, "heat conversion  factor", QString::fromStdString(settings.conversion));
  addValue(settingsGroup, "surface weighting  factor", QString::fromStdString(settings.weight));
  addValue(settingsGroup, "differential  tangential velocity", QString::fromStdString(settings.velocity));
  addValue(settingsGroup, "mu", QString::fromStdString(settings.mu));
  addValue(settingsGroup, "lambda", QString::fromStdString(settings.lambdaValue));
  if (!settings.pressureOverclosure.empty())
  {
    addTable("pressure_overclosure", "Pressure / overclosure", {"Pressure", "Overclosure"}, settings.pressureOverclosure);
  }
  if (!settings.conductance.empty())
  {
    addTable("gap_conductance", "Gap conductance", {"Conductance", "Contact pressure", "Temperature"}, settings.conductance);
  }
  return true;
}

void SurfaceInteractionsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXInteractionsInterface core;
  SurfaceInteractionsSettings settings;
  std::string error;
  if (!core.get_surfaceinteraction_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify surfaceinteraction %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setSurfaceInteraction(id); });
}
