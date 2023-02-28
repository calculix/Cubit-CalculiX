#include "HistoryOutputsTree.hpp"
#include "CalculiXCoreInterface.hpp"

HistoryOutputsTree::HistoryOutputsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

HistoryOutputsTree::~HistoryOutputsTree()
{
}

void HistoryOutputsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"History Outputs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void HistoryOutputsTree::update()
{
  std::vector<std::vector<std::string>> outputs_tree_data;
  outputs_tree_data = ccx_iface->get_historyoutputs_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < outputs_tree_data.size(); i++)
  {
    // check if output already exists as item
    
    ChildId = this->get_child_id(outputs_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addOutput(QString::fromStdString(outputs_tree_data[i][0]),
        QString::fromStdString(outputs_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = outputs_tree_data[i][1];    

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

    for (size_t ii = 0; ii < outputs_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==outputs_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeOutput(temp_child);
    }
  }
}

void HistoryOutputsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void HistoryOutputsTree::addOutput(QString output_id, QString output_name)
{
  QTreeWidgetItem *OutputTreeChild = new QTreeWidgetItem();
  
  OutputTreeChild->setText(0, output_name);
    
  OutputTreeChild->setText(1, output_id);

  this->addChild(OutputTreeChild);
}

void HistoryOutputsTree::removeOutput(QTreeWidgetItem *output)
{
  this->removeChild(output);
  delete output;
}

int HistoryOutputsTree::get_child_id(std::string output_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==output_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}