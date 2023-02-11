
#include "MaterialTree.hpp"
#include "CalculiXCoreInterface.hpp"

MaterialTree::MaterialTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

MaterialTree::~MaterialTree()
{
}

void MaterialTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Material");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void MaterialTree::update()
{
  std::vector<std::vector<std::string>> material_tree_data;
  material_tree_data = ccx_iface->get_material_tree_data(); 

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
    temp_child = this->child(i-1);

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
  QTreeWidgetItem *MaterialTreeChild = new QTreeWidgetItem();
  
  MaterialTreeChild->setText(0, material_name);
    
  MaterialTreeChild->setText(1, material_id);

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
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==material_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}