#include "MaterialTree.hpp"

#include "GUI/application/ModelTree.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "CubitMessage.hpp"
#include "MaterialManagement.hpp"
#include <QMenu>
#include <QAction>
MaterialTree::MaterialTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

MaterialTree::~MaterialTree()
{
}

void MaterialTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  if (isInitialized)
  {
    return;
  }

  QTreeWidgetItem::setText(0,"Material");
  this->setIcon(0,ccx_gui_resources_iface.getIcon2("MaterialTree"));
  isInitialized = true;
}

void MaterialTree::update()
{
  CalculiXMaterialsInterface ccx_materials_iface;

  std::vector<std::vector<std::string>> material_tree_data;
  material_tree_data = ccx_materials_iface.get_material_tree_data();

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < material_tree_data.size(); i++)
  {
    // check if material already exists as item
    
    ChildId = this->get_child_id(material_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addMaterial(QString::fromStdString(material_tree_data[i][0]),
        QString::fromStdString(material_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = material_tree_data[i][1];    

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

    for (size_t ii = 0; ii < material_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==material_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeMaterial(temp_child);
    }
  }
}

void MaterialTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void MaterialTree::addMaterial(QString material_id, QString material_name)
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  QTreeWidgetItem *MaterialTreeChild = new QTreeWidgetItem();
  
  MaterialTreeChild->setText(0, material_name);
    
  MaterialTreeChild->setText(1, material_id);

  MaterialTreeChild->setIcon(0,ccx_gui_resources_iface.getIcon2("MaterialTree"));

  this->addChild(MaterialTreeChild);
}

void MaterialTree::removeMaterial(QTreeWidgetItem *material)
{
  this->removeChild(material);
  delete material;
}

int MaterialTree::get_child_id(std::string material_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==material_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void MaterialTree::populateContextMenu(
  QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host)
{
  if (clickedItem == nullptr)
  {
    return;
  }

  if (clickedItem == this)
  {
    {
      QAction *action = menu.addAction("Open Material Management");
      ModelTree *hostPtr = &host;
      QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
      {
        if (hostPtr->myMaterialManagement == nullptr)
        {
          PRINT_WARNING("The requested model tree service is unavailable.\n");
          return;
        }

        hostPtr->myMaterialManagement->show();
      });
    }
    return;
  }

  if (clickedItem->parent() != this)
  {
    return;
  }

  {
    QAction *action = menu.addAction("Open Material Management");
    ModelTree *hostPtr = &host;
    QObject::connect(action, &QAction::triggered, hostPtr, [hostPtr]()
    {
      if (hostPtr->myMaterialManagement == nullptr)
      {
        PRINT_WARNING("The requested model tree service is unavailable.\n");
        return;
      }

      hostPtr->myMaterialManagement->show();
    });
  }
}
