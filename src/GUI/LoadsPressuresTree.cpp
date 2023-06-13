#include "LoadsPressuresTree.hpp"
#include "CalculiXCoreInterface.hpp"

LoadsPressuresTree::LoadsPressuresTree(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

LoadsPressuresTree::~LoadsPressuresTree()
{
}

void LoadsPressuresTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Pressures");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("LoadsPressuresTree"));
  isInitialized = true;
}

void LoadsPressuresTree::update()
{
  std::vector<std::vector<std::string>> loads_tree_data;
  loads_tree_data = ccx_iface->get_loadspressures_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < loads_tree_data.size(); i++)
  {
    // check if load already exists as item
    
    ChildId = this->get_child_id(loads_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addLoad(QString::fromStdString(loads_tree_data[i][0]),
        QString::fromStdString(loads_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = loads_tree_data[i][1];    

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

    for (size_t ii = 0; ii < loads_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==loads_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeLoad(temp_child);
    }
  }
}

void LoadsPressuresTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void LoadsPressuresTree::addLoad(QString load_id, QString load_name)
{
  QTreeWidgetItem *LoadTreeChild = new QTreeWidgetItem();
  
  LoadTreeChild->setText(0, load_name);
    
  LoadTreeChild->setText(1, load_id);

  LoadTreeChild->setIcon(0,ccx_iface->getIcon2("LoadsPressuresTree"));

  this->addChild(LoadTreeChild);
}

void LoadsPressuresTree::removeLoad(QTreeWidgetItem *load)
{
  this->removeChild(load);
  delete load;
}

int LoadsPressuresTree::get_child_id(std::string load_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==load_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}