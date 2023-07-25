#include "StepsTree.hpp"
#include "StepsLoadsTree.hpp"
#include "StepsBCsTree.hpp"
#include "StepsHistoryOutputsTree.hpp"
#include "StepsFieldOutputsTree.hpp"
#include "CalculiXCoreInterface.hpp"

StepsTree::StepsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

StepsTree::~StepsTree()
{}

void StepsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Steps");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->setIcon(0,ccx_iface->getIcon2("StepsTree")); 
  isInitialized = true;
}

void StepsTree::update()
{
  std::vector<std::vector<std::string>> steps_tree_data;
  steps_tree_data = ccx_iface->get_steps_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < steps_tree_data.size(); i++)
  {
    // check if step already exists as item
    
    ChildId = this->get_child_id(steps_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addStep(QString::fromStdString(steps_tree_data[i][0]),
        QString::fromStdString(steps_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      // update childs
      StepsLoadsTree *StepsLoadsChild;
      StepsLoadsChild = dynamic_cast<StepsLoadsTree*>(temp_child->child(0));
      StepsLoadsChild->update();
      StepsBCsTree *StepsBCsChild;
      StepsBCsChild = dynamic_cast<StepsBCsTree*>(temp_child->child(1));
      StepsBCsChild->update();
      StepsHistoryOutputsTree *StepsHistoryOutputsChild;
      StepsHistoryOutputsChild = dynamic_cast<StepsHistoryOutputsTree*>(temp_child->child(2));
      StepsHistoryOutputsChild->update();
      StepsFieldOutputsTree *StepsFieldOutputsChild;
      StepsFieldOutputsChild = dynamic_cast<StepsFieldOutputsTree*>(temp_child->child(3));
      StepsFieldOutputsChild->update();

      str_check = steps_tree_data[i][1];    

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

    for (size_t ii = 0; ii < steps_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==steps_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeStep(temp_child);
    }
  }
}

void StepsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void StepsTree::addStep(QString step_id, QString step_name)
{
  QTreeWidgetItem *StepTreeChild = new QTreeWidgetItem();
  
  StepTreeChild->setText(0, step_name);
    
  StepTreeChild->setText(1, step_id);

  StepTreeChild->setIcon(0,ccx_iface->getIcon2("StepsTree")); 

  StepsLoadsTree *StepsLoadsChild = new StepsLoadsTree(StepTreeChild);
  StepsLoadsChild->initialize(step_id.toInt());

  StepsBCsTree *StepsBCsChild = new StepsBCsTree(StepTreeChild);
  StepsBCsChild->initialize(step_id.toInt());

  StepsHistoryOutputsTree *StepsHistoryOutputsChild = new StepsHistoryOutputsTree(StepTreeChild);
  StepsHistoryOutputsChild->initialize(step_id.toInt());

  StepsFieldOutputsTree *StepsFieldOutputsChild = new StepsFieldOutputsTree(StepTreeChild);
  StepsFieldOutputsChild->initialize(step_id.toInt());

  this->addChild(StepTreeChild);
}

void StepsTree::removeStep(QTreeWidgetItem *step)
{
  this->removeChild(step);
  delete step;
}

int StepsTree::get_child_id(std::string step_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==step_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}