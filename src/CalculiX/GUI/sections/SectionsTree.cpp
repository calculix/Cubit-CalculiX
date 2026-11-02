#include "GUI/properties/PropertySelection.hpp"
#include <QAction>
#include "SectionsDeletePanel.hpp"
#include "SectionsModifyMembranePanel.hpp"
#include "SectionsModifyBeamPanel.hpp"
#include "SectionsModifyShellPanel.hpp"
#include "SectionsModifySolidPanel.hpp"
#include "CubitMessage.hpp"
#include "TaskPanel.hpp"
#include "SectionsTree.hpp"

#include "GUI/application/ModelTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include <QMenu>
SectionsTree::SectionsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

SectionsTree::~SectionsTree()
{
}

void SectionsTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Sections");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("SectionsTree"));
  isInitialized = true;
}

void SectionsTree::update()
{
  CalculiXSectionsInterface ccx_sections_iface;

  std::vector<std::vector<std::string>> sections_tree_data;
  sections_tree_data = ccx_sections_iface.get_sections_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < sections_tree_data.size(); i++)
  {
    // check if section already exists as item
    
    ChildId = this->get_child_id(sections_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addSection(QString::fromStdString(sections_tree_data[i][0]),
        QString::fromStdString(sections_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = sections_tree_data[i][1];    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }

  // check if item has been removed

  for (size_t i = this->childCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = this->child(int(i)-1);

    for (size_t ii = 0; ii < sections_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==sections_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeSection(temp_child);
    }
  }
}

void SectionsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void SectionsTree::addSection(QString section_id, QString section_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *SectionTreeChild = new QTreeWidgetItem();
  
  SectionTreeChild->setText(0, section_name);
    
  SectionTreeChild->setText(1, section_id);
  SectionTreeChild->setData(0, PropertyFeatureRole, static_cast<int>(PropertyFeature::Section));
  SectionTreeChild->setData(0, PropertyIdRole, section_id.toInt());

  SectionTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("SectionsTree"));

  this->addChild(SectionTreeChild);
}

void SectionsTree::removeSection(QTreeWidgetItem *section)
{
  this->removeChild(section);
  delete section;
}

int SectionsTree::get_child_id(std::string section_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==section_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void SectionsTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  int selectedId = 0;
  if (clickedItem != this && !readItemId(clickedItem, selectedId))
  {
    return;
  }

  if (clickedItem == this)
  {
    addPanelAction(menu, "Create Section", host, "CCXSectionsCreate");
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  addPanelAction(menu, "Create Section", host, "CCXSectionsCreate");
  {
    auto* action = menu.addAction("Modify Section");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      SectionsTree::open_modify(selectedId, host);
    });
  }
  {
    auto* action = menu.addAction("Delete Section");
    QObject::connect(action, &QAction::triggered, &menu, [selectedId, &host]()
    {
      SectionsTree::open_delete(selectedId, host);
    });
  }
}


void SectionsTree::open_modify(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  CalculiXSectionsInterface core;
  SectionsSettings settings;
  std::string error;
  if (!core.get_section_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot open Section %d: %s\n", id, error.c_str());
    return;
  }

  QString marker;
  switch (settings.type)
  {
    case 1: marker = "CCXSectionsModifySolid"; break;
    case 2: marker = "CCXSectionsModifyShell"; break;
    case 3: marker = "CCXSectionsModifyBeam"; break;
    case 4: marker = "CCXSectionsModifyMembrane"; break;
    default:
      PRINT_ERROR("This stored type has no modify panel.\n");
      return;
  }
  host.setWidgetInCmdPanelMarker(marker);

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<SectionsModifySolidPanel*>(widget))
    {
      if (settings.type == 1)
      {
        panel->setSection(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<SectionsModifyShellPanel*>(widget))
    {
      if (settings.type == 2)
      {
        panel->setSection(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<SectionsModifyBeamPanel*>(widget))
    {
      if (settings.type == 3)
      {
        panel->setSection(id);
        return;
      }
    }
    if (auto* panel = dynamic_cast<SectionsModifyMembranePanel*>(widget))
    {
      if (settings.type == 4)
      {
        panel->setSection(id);
        return;
      }
    }
  }
  PRINT_ERROR("Unable to open the Section modify panel.\n");
}

void SectionsTree::open_delete(int id, ModelTree& host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXSectionsDelete");

  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    if (auto* panel = dynamic_cast<SectionsDeletePanel*>(widget))
    {
      panel->setSection(id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the Section delete panel.\n");
}

void SectionsTree::activate(QTreeWidgetItem* item, ModelTree& host)
{
  if (item == nullptr)
  {
    return;
  }
  if (item == this)
  {
    host.setWidgetInCmdPanelMarker("CCXSectionsCreate");
    return;
  }

  if (item->parent() != this)
  {
    return;
  }

  int id = -1;
  if (!readItemId(item, id))
  {
    return;
  }
  open_modify(id, host);
}
