
#include "SectionsTree.hpp"
#include "CalculiXCoreInterface.hpp"

SectionsTree::SectionsTree(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

SectionsTree::~SectionsTree()
{
}

void SectionsTree::initialize()
{
  if(isInitialized)
    return;

  QTreeWidgetItem::setText(0,"Sections");
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  
  isInitialized = true;
}

void SectionsTree::update()
{
  std::vector<std::vector<std::string>> sections_tree_data;
  sections_tree_data = ccx_iface->get_sections_tree_data(); 

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
    temp_child = this->child(i-1);

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
  QTreeWidgetItem *SectionTreeChild = new QTreeWidgetItem();
  
  SectionTreeChild->setText(0, section_name);
    
  SectionTreeChild->setText(1, section_id);

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
    
    temp_child = this->child(i);

    if (temp_child->text(1).toStdString()==section_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}