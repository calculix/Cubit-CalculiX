#include "DampingPropertiesWidget.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"

DampingPropertiesWidget::DampingPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool DampingPropertiesWidget::setDamping(int id)
{
  clear();
  CalculiXStepsInterface core;
  DampingSettings settings;
  std::string error;
  if (!core.get_damping_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Damping properties %d: %s\n", id, error.c_str());
    return false;
  }
  if (id < 0 || id > 2)
  {
    PRINT_ERROR("Invalid global property selection.\n");
    return false;
  }
  addIdentity("Damping", -1);
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  if (id == 0 || id == 1)
  {
    addValue(settingsGroup, "Alpha", QString::fromStdString(settings.alpha));
  }
  if (id == 0 || id == 2)
  {
    addValue(settingsGroup, "Beta", QString::fromStdString(settings.beta));
  }
  return true;
}
