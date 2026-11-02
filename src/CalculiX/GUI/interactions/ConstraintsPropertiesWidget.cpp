#include "ConstraintsPropertiesWidget.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "CubitMessage.hpp"

ConstraintsPropertiesWidget::ConstraintsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool ConstraintsPropertiesWidget::setConstraint(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXInteractionsInterface core;
  ConstraintsSettings settings;
  std::string error;
  if (!core.get_constraint_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Constraint properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  if (settings.type == 2 || settings.type == 3)
  {
    auto* general = addIdentity("Constraint", settings.id);
    addEditableName(general, id, QString::fromStdString(settings.name));
  }
  else
  {
    addIdentity("Constraint", settings.id, settings.name);
  }
  auto* typeGroup = addGroup("classification", "Classification", PropertySection::Classification);
  switch (settings.type)
  {
    case 1:
    {
      addValue(typeGroup, "Subtype", "Rigid body");
      auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
      addReference(assignment, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.block));
      addReference(assignment, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.nodeset));
      addReference(assignment, PropertyReferenceKind::Vertex, "Reference vertex", QString::fromStdString(settings.reference));
      addReference(assignment, PropertyReferenceKind::Vertex, "Rotation vertex", QString::fromStdString(settings.rotation));
      break;
    }
    case 2:
    {
      addValue(typeGroup, "Subtype", "Tie");
      auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
      addReference(assignment, PropertyReferenceKind::Sideset, "Master sideset", QString::fromStdString(settings.master));
      addReference(assignment, PropertyReferenceKind::Sideset, "Slave sideset", QString::fromStdString(settings.slave));
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addValue(settingsGroup, "Position Tolerance", QString::fromStdString(settings.tolerance));
      break;
    }
    case 3:
    {
      addValue(typeGroup, "Subtype", "Equation");
      break;
    }
    case 4:
    {
      addValue(typeGroup, "Subtype", "Equation group");
      break;
    }
    default:
      PRINT_ERROR("Unknown stored property subtype.\n");
      clear();
      return false;
  }
  if (!settings.terms.empty())
  {
    addTable("equation_terms", "Equation terms", {"Node ID", "DOF", "Coefficient"}, settings.terms);
  }
  if (!settings.equationPairs.empty())
  {
    addTable("equation_pairs", "Equation pairs", {"Node 1", "Node 2", "DOF"}, settings.equationPairs);
  }
  return true;
}

void ConstraintsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXInteractionsInterface core;
  ConstraintsSettings settings;
  std::string error;
  if (!core.get_constraint_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  QString subtype;
  switch (settings.type)
  {
    case 2: subtype = "tie"; break;
    case 3: subtype = "equation"; break;
    default: return;
  }
  const QString command = QString("ccx modify constraint %1 %2 name %3").arg(subtype).arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setConstraint(id); });
}
