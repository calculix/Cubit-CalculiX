#include "CustomLinesTree.hpp"
#include "CalculiXCoreInterface.hpp"

CustomLinesTree::CustomLinesTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

CustomLinesTree::~CustomLinesTree()
{
}

void CustomLinesTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Custom Lines");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("CustomLinesTree")); 
  isInitialized = true;
}

void CustomLinesTree::update()
{
  std::vector<std::vector<std::string>> customlines_tree_data;
  customlines_tree_data = ccx_iface->get_customlines_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < customlines_tree_data.size(); i++)
  {
    // check if customline already exists as item
    
    ChildId = this->get_child_id(customlines_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addCustomLine(QString::fromStdString(customlines_tree_data[i][0]),
        QString::fromStdString(customlines_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = customlines_tree_data[i][1];    

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

    for (size_t ii = 0; ii < customlines_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==customlines_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeCustomLine(temp_child);
    }
  }
}

void CustomLinesTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void CustomLinesTree::addCustomLine(QString customline_id, QString customline_name)
{
  QTreeWidgetItem *CustomLineTreeChild = new QTreeWidgetItem();
  
  CustomLineTreeChild->setText(0, customline_name);
    
  CustomLineTreeChild->setText(1, customline_id);

  CustomLineTreeChild->setIcon(0,ccx_iface->getIcon2("CustomLinesTree"));

  this->addChild(CustomLineTreeChild);
}

void CustomLinesTree::removeCustomLine(QTreeWidgetItem *customline)
{
  this->removeChild(customline);
  delete customline;
}

int CustomLinesTree::get_child_id(std::string customline_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(int(i));

    if (temp_child->text(1).toStdString()==customline_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}