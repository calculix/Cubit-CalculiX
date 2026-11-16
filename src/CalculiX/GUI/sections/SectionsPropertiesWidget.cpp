#include "GUI/common/CalculiXUnitPresentation.hpp"
#include "SectionsPropertiesWidget.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "CubitMessage.hpp"

SectionsPropertiesWidget::SectionsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool SectionsPropertiesWidget::setSection(int id)
{
  clear();
  CalculiXSectionsInterface core;
  SectionsSettings settings;
  std::string error;
  if (!core.get_section_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Section properties %d: %s\n", id, error.c_str());
    return false;
  }
  addIdentity("Section", settings.id);
  auto* typeGroup = addGroup("classification", "Classification", PropertySection::Classification);
  switch (settings.type)
  {
    case 1:
    {
      addValue(typeGroup, "Subtype", "Solid");
      auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
      addReference(assignment, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.block));
      addReference(assignment, PropertyReferenceKind::Material, "Material", QString::fromStdString(settings.material));
      addReference(assignment, PropertyReferenceKind::Orientation, "Orientation", QString::fromStdString(settings.orientation));
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addValue(settingsGroup, CalculiXUnitPresentation::title("Thickness", "length"), QString::fromStdString(settings.thickness));
      break;
    }
    case 2:
    {
      addValue(typeGroup, "Subtype", "Shell");
      auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
      addReference(assignment, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.block));
      addReference(assignment, PropertyReferenceKind::Material, "Material", QString::fromStdString(settings.material));
      addReference(assignment, PropertyReferenceKind::Orientation, "Orientation", QString::fromStdString(settings.orientation));
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addValue(settingsGroup, CalculiXUnitPresentation::title("Thickness", "length"), QString::fromStdString(settings.thickness));
      addValue(settingsGroup, "Offset", QString::fromStdString(settings.offset));
      break;
    }
    case 3:
    {
      addValue(typeGroup, "Subtype", "Beam");
      auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
      addReference(assignment, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.block));
      addReference(assignment, PropertyReferenceKind::Material, "Material", QString::fromStdString(settings.material));
      addReference(assignment, PropertyReferenceKind::Orientation, "Orientation", QString::fromStdString(settings.orientation));
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addValue(settingsGroup, "beam Type", QString::fromStdString(settings.beamType));
      addValue(settingsGroup, CalculiXUnitPresentation::title("Parameter 1", "length"), QString::fromStdString(settings.parameter1));
      addValue(settingsGroup, CalculiXUnitPresentation::title("Parameter 2", "length"), QString::fromStdString(settings.parameter2));
      addValue(settingsGroup, CalculiXUnitPresentation::title("Parameter 3", "length"), QString::fromStdString(settings.parameter3));
      addValue(settingsGroup, CalculiXUnitPresentation::title("Parameter 4", "length"), QString::fromStdString(settings.parameter4));
      addValue(settingsGroup, CalculiXUnitPresentation::title("Parameter 5", "length"), QString::fromStdString(settings.parameter5));
      addValue(settingsGroup, CalculiXUnitPresentation::title("Parameter 6", "length"), QString::fromStdString(settings.parameter6));
      addValue(settingsGroup, "x", QString::fromStdString(settings.x));
      addValue(settingsGroup, "y", QString::fromStdString(settings.y));
      addValue(settingsGroup, "z", QString::fromStdString(settings.z));
      addValue(settingsGroup, "Offset 1", QString::fromStdString(settings.offset1));
      addValue(settingsGroup, "Offset 2", QString::fromStdString(settings.offset2));
      break;
    }
    case 4:
    {
      addValue(typeGroup, "Subtype", "Membrane");
      auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
      addReference(assignment, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.block));
      addReference(assignment, PropertyReferenceKind::Material, "Material", QString::fromStdString(settings.material));
      addReference(assignment, PropertyReferenceKind::Orientation, "Orientation", QString::fromStdString(settings.orientation));
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addValue(settingsGroup, CalculiXUnitPresentation::title("Thickness", "length"), QString::fromStdString(settings.thickness));
      addValue(settingsGroup, "Offset", QString::fromStdString(settings.offset));
      break;
    }
    default:
      PRINT_ERROR("Unknown stored property subtype.\n");
      clear();
      return false;
  }
  return true;
}
