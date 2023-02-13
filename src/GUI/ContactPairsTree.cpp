#include "ContactPairsTree.hpp"
#include "CalculiXCoreInterface.hpp"

ContactPairsTree::ContactPairsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

ContactPairsTree::~ContactPairsTree()
{
}

void ContactPairsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Contact Pairs");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void ContactPairsTree::update()
{
  std::vector<std::vector<std::string>> contactpairs_tree_data;
  contactpairs_tree_data = ccx_iface->get_contactpairs_tree_data(); 

  QTreeWidgetItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < contactpairs_tree_data.size(); i++)
  {
    // check if contactpair already exists as item
    
    ChildId = this->get_child_id(contactpairs_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addContactPair(QString::fromStdString(contactpairs_tree_data[i][0]),
        QString::fromStdString(contactpairs_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = this->child(ChildId);
      
      str_check = contactpairs_tree_data[i][1];    

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

    for (size_t ii = 0; ii < contactpairs_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==contactpairs_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      removeContactPair(temp_child);
    }
  }
}

void ContactPairsTree::clear()
{
  // Remove all of our tree items.
  while (this->childCount()>0)
  {
    this->removeChild(this->child(0));
  }
}

void ContactPairsTree::addContactPair(QString contactpair_id, QString contactpair_name)
{
  QTreeWidgetItem *ContactPairTreeChild = new QTreeWidgetItem();
  
  ContactPairTreeChild->setText(0, contactpair_name);
    
  ContactPairTreeChild->setText(1, contactpair_id);

  this->addChild(ContactPairTreeChild);
}

void ContactPairsTree::removeContactPair(QTreeWidgetItem *contactpair)
{
  this->removeChild(contactpair);
  delete contactpair;
}

int ContactPairsTree::get_child_id(std::string contactpair_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;

  for (size_t i = 0; i < this->childCount(); i++)
  {
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==contactpair_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}