#include "OrientationsTree.hpp"
#include "CalculiXCoreInterface.hpp"

OrientationsTree::OrientationsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

OrientationsTree::~OrientationsTree()
{
}

void OrientationsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Orientations");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("OrientationsTree")); 
  isInitialized = true;
}

void OrientationsTree::update()
{
  std::vector<std::vector<std::string>> orientations_tree_data;
  orientations_tree_data = ccx_iface->get_orientations_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < orientations_tree_data.size(); i++)
  {
    // check if orientation already exists as item
    
    ChildId = this->get_child_id(orientations_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addOrientation(QString::fromStdString(orientations_tree_data[i][0]),
        QString::fromStdString(orientations_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = orientations_tree_data[i][1];    

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

    for (size_t ii = 0; ii < orientations_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==orientations_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeOrientation(temp_child);
    }
  }
}

void OrientationsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void OrientationsTree::addOrientation(QString orientation_id, QString name)
{
  QTreeWidgetItem *OrientationTreeChild = new QTreeWidgetItem();
  
  OrientationTreeChild->setText(0, name);
    
  OrientationTreeChild->setText(1, orientation_id);

  OrientationTreeChild->setIcon(0,ccx_iface->getIcon2("OrientationsTree"));

  this->addChild(OrientationTreeChild);
}

void OrientationsTree::removeOrientation(QTreeWidgetItem *orientation)
{
  this->removeChild(orientation);
  delete orientation;
}

int OrientationsTree::get_child_id(std::string orientation_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==orientation_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}