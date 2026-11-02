#include "ContactPairsPropertiesWidget.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "CubitMessage.hpp"

ContactPairsPropertiesWidget::ContactPairsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool ContactPairsPropertiesWidget::setContactPair(int id)
{
  clear();
  CalculiXInteractionsInterface core;
  ContactPairsSettings settings;
  std::string error;
  if (!core.get_contactpair_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load ContactPair properties %d: %s\n", id, error.c_str());
    return false;
  }
  addIdentity("Contact Pair", settings.id);
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::SurfaceInteraction, "Surface interaction", QString::fromStdString(settings.surfaceInteraction));
  addReference(assignment, PropertyReferenceKind::Sideset, "Master sideset", QString::fromStdString(settings.master));
  addReference(assignment, PropertyReferenceKind::Sideset, "Slave sideset", QString::fromStdString(settings.slave));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Type", QString::fromStdString(settings.contactType));
  addValue(settingsGroup, "Adjust", QString::fromStdString(settings.adjust));
  addReference(settingsGroup, PropertyReferenceKind::Nodeset, "Adjust nodeset", QString::fromStdString(settings.adjustNodeset));
  return true;
}
