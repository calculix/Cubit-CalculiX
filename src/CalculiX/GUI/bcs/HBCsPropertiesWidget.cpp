#include "HBCsPropertiesWidget.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "CubitMessage.hpp"

HBCsPropertiesWidget::HBCsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool HBCsPropertiesWidget::setHBCs(int id)
{
  clear();
  if (id != 0)
  {
    PRINT_ERROR("The homogeneous BC collection has ID 0.\n");
    return false;
  }
  CalculiXBCsInterface core;
  addIdentity("Homogeneous boundary conditions", id);
  auto* displacements = addGroup("displacements", "Displacements", PropertySection::Data);
  for (const auto& row : core.get_hbcsdisplacements_tree_data())
  {
    if (row.size() >= 2)
    {
      addValue(displacements, QString::fromStdString(row[0]), QString::fromStdString(row[1]));
    }
  }
  auto* temperatures = addGroup("temperatures", "Temperatures", PropertySection::Data);
  for (const auto& row : core.get_hbcstemperatures_tree_data())
  {
    if (row.size() >= 2)
    {
      addValue(temperatures, QString::fromStdString(row[0]), QString::fromStdString(row[1]));
    }
  }
  return true;
}
