#include "GUI/common/CalculiXUnitPresentation.hpp"
#include "LoadsForcesPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsForcesPropertiesWidget::LoadsForcesPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsForcesPropertiesWidget::setForce(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsForcesSettings settings;
  std::string error;
  if (!core.get_force_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Force properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Force", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  if (settings.referencePoint > 0)
  {
    addValue(assignment, "Reference point", QString::number(settings.referencePoint));
  }
  else
  {
    addReference(assignment, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.nodeset));
  }
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, CalculiXUnitPresentation::title("Force X", "force"), QString::fromStdString(settings.component1));
  addValue(components, CalculiXUnitPresentation::title("Force Y", "force"), QString::fromStdString(settings.component2));
  addValue(components, CalculiXUnitPresentation::title("Force Z", "force"), QString::fromStdString(settings.component3));
  addValue(components, CalculiXUnitPresentation::title("Moment X", "moment"), QString::fromStdString(settings.component4));
  addValue(components, CalculiXUnitPresentation::title("Moment Y", "moment"), QString::fromStdString(settings.component5));
  addValue(components, CalculiXUnitPresentation::title("Moment Z", "moment"), QString::fromStdString(settings.component6));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, CalculiXUnitPresentation::title("Time Delay", "time"), QString::fromStdString(settings.timeDelay));
  return true;
}

void LoadsForcesPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsForcesSettings settings;
  std::string error;
  if (!core.get_force_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify force %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setForce(id); });
}
