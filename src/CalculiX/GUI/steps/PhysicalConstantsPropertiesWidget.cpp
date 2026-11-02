#include "PhysicalConstantsPropertiesWidget.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"

PhysicalConstantsPropertiesWidget::PhysicalConstantsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool PhysicalConstantsPropertiesWidget::setPhysicalConstants(int id)
{
  clear();
  CalculiXStepsInterface core;
  PhysicalConstantsSettings settings;
  std::string error;
  if (!core.get_physicalconstants_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load PhysicalConstants properties %d: %s\n", id, error.c_str());
    return false;
  }
  if (id < 0 || id > 3)
  {
    PRINT_ERROR("Invalid global property selection.\n");
    return false;
  }
  addIdentity("Physical Constants", -1);
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  if (id == 0 || id == 1)
  {
    addValue(settingsGroup, "Absolute Zero", QString::fromStdString(settings.absoluteZero));
  }
  if (id == 0 || id == 2)
  {
    addValue(settingsGroup, "Stefan Boltzmann", QString::fromStdString(settings.stefanBoltzmann));
  }
  if (id == 0 || id == 3)
  {
    addValue(settingsGroup, "Newton Gravity", QString::fromStdString(settings.newtonGravity));
  }
  return true;
}
