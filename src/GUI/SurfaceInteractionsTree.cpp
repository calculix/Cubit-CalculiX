
#include "SurfaceInteractionsTree.hpp"
#include "CalculiXCoreInterface.hpp"

SurfaceInteractionsTree::SurfaceInteractionsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

SurfaceInteractionsTree::~SurfaceInteractionsTree()
{
}

void SurfaceInteractionsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Surface Interactions");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void SurfaceInteractionsTree::update()
{
  std::vector<std::vector<std::string>> surfaceinteractions_tree_data;
  surfaceinteractions_tree_data = ccx_iface->get_surfaceinteractions_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < surfaceinteractions_tree_data.size(); i++)
  {
    // check if surfaceinteraction already exists as item
    
    ChildId = this->get_child_id(surfaceinteractions_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addSurfaceInteraction(QString::fromStdString(surfaceinteractions_tree_data[i][0]),
        QString::fromStdString(surfaceinteractions_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = surfaceinteractions_tree_data[i][1];    

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

    for (size_t ii = 0; ii < surfaceinteractions_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==surfaceinteractions_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeSurfaceInteraction(temp_child);
    }
  }
}

void SurfaceInteractionsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void SurfaceInteractionsTree::addSurfaceInteraction(QString surfaceinteraction_id, QString surfaceinteraction_name)
{
  QTreeWidgetItem *SurfaceInteractionTreeChild = new QTreeWidgetItem();
  
  SurfaceInteractionTreeChild->setText(0, surfaceinteraction_name);
    
  SurfaceInteractionTreeChild->setText(1, surfaceinteraction_id);

  this->addChild(SurfaceInteractionTreeChild);
}

void SurfaceInteractionsTree::removeSurfaceInteraction(QTreeWidgetItem *surfaceinteraction)
{
  this->removeChild(surfaceinteraction);
  delete surfaceinteraction;
}

int SurfaceInteractionsTree::get_child_id(std::string surfaceinteraction_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==surfaceinteraction_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}