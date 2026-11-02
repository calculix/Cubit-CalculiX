#include "InitialConditionsPropertiesWidget.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"

InitialConditionsPropertiesWidget::InitialConditionsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool InitialConditionsPropertiesWidget::setInitialCondition(int id)
{
  clear();
  CalculiXStepsInterface core;
  InitialConditionsSettings settings;
  std::string error;
  if (!core.get_initialcondition_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load InitialCondition properties %d: %s\n", id, error.c_str());
    return false;
  }
  addIdentity("Initial Condition", settings.id);
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Type", QString::fromStdString(settings.conditionType));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  if (settings.type == 1 || settings.type == 2)
  {
    const auto kind = settings.type == 1 ? PropertyReferenceKind::Displacement : PropertyReferenceKind::Temperature;
    addReference(assignment, kind, "BC", QString::fromStdString(settings.bcId));
  }
  if (!settings.blockStress.empty())
  {
    addTable("block_stresses", "Block stresses", {"Block ID", "Sxx", "Syy", "Szz", "Sxy", "Sxz", "Syz"}, settings.blockStress);
  }
  if (!settings.elementStress.empty())
  {
    addTable("element_stresses", "Element stresses", {"Element ID", "Integration point", "Sxx", "Syy", "Szz", "Sxy", "Sxz", "Syz"}, settings.elementStress);
  }
  return true;
}
