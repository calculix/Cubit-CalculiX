#include "HBCsManagement.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"

HBCsManagement::HBCsManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  this->setGeometry(0,0,800,800);
  this->setWindowTitle("homogeneous Boundary Conditions Management");

  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_hbcs_1 = new QVBoxLayout();
  boxLayout_assignment_1 = new QVBoxLayout();
  boxLayout_assignment_2 = new QVBoxLayout();
  boxLayout_assignment_3 = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,1,2,1,2, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_hbcs_1,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_assignment_1,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_assignment_2,0,2, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_assignment_3,0,3, Qt::AlignTop);
  boxLayout_assignment_2->addSpacing(30);

  // buttons
  pushButton_ok = new QPushButton();
  //pushButton_ok->setGeometry(310,530,75,23);
  pushButton_ok->setText("OK");
  boxLayout_window->addWidget(pushButton_ok);

  pushButton_apply = new QPushButton();
  //pushButton_apply->setGeometry(400,530,75,23);
  pushButton_apply->setText("Apply");
  boxLayout_window->addWidget(pushButton_apply);

  pushButton_close = new QPushButton();
  //pushButton_close->setGeometry(490,530,75,23);
  pushButton_close->setText("Close");
  boxLayout_window->addWidget(pushButton_close);
    
  pushButton_add = new QPushButton();
  //pushButton_add->setGeometry(420,70,21,21);
  pushButton_add->setText(">");
  boxLayout_assignment_2->addWidget(pushButton_add);

  pushButton_remove = new QPushButton();
  //pushButton_remove->setGeometry(420,100,21,21);
  pushButton_remove->setText("<");
  boxLayout_assignment_2->addWidget(pushButton_remove);

  // labels
  label_hbcs = new QLabel();
  //label_material->setGeometry(10,10,80,16);
  label_hbcs->setText("hbcs");
  boxLayout_hbcs_1->addWidget(label_hbcs);

  label_available = new QLabel();
  //label_available->setGeometry(300,10,100,16);
  label_available->setText("Available");
  boxLayout_assignment_1->addWidget(label_available);

  label_used = new QLabel();
  //label_used->setGeometry(450,10,100,16);
  label_used->setText("Used");
  boxLayout_assignment_3->addWidget(label_used);

  // tree/lists
  tree_hbcs = new QTreeWidget();
  //tree_material->setGeometry(10,30,181,191);
  tree_hbcs->setColumnCount(2);
  tree_hbcs->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_hbcs_1->addWidget(tree_hbcs);

  tree_available = new QTreeWidget();
  tree_available->setColumnCount(2);
  tree_available->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_assignment_1->addWidget(tree_available);

  tree_used = new QTreeWidget();
  tree_used->setColumnCount(2);
  tree_used->setHeaderLabels(QStringList() << "Name" << "ID");
  //list_available->setGeometry(300,30,111,191);
  boxLayout_assignment_3->addWidget(tree_used);

  tree_available_bcs = new QTreeWidgetItem(tree_available);
  tree_available_bcs->setText(0,"BCs");
  tree_available_bcs->setIcon(0,ccx_iface->getIcon2("HBCsTree"));
  tree_available_bcs_displacements = new QTreeWidgetItem(tree_available_bcs);
  tree_available_bcs_displacements->setText(0,"Displacements");
  tree_available_bcs_displacements->setIcon(0,ccx_iface->getIcon2("HBCsDisplacementsTree"));
  tree_available_bcs_temperatures = new QTreeWidgetItem(tree_available_bcs);
  tree_available_bcs_temperatures->setText(0,"Temperatures");
  tree_available_bcs_temperatures->setIcon(0,ccx_iface->getIcon2("HBCsTemperaturesTree"));

  tree_used_bcs = new QTreeWidgetItem(tree_used);
  tree_used_bcs->setText(0,"BCs");
  tree_used_bcs->setIcon(0,ccx_iface->getIcon2("HBCsTree"));
  tree_used_bcs_displacements = new QTreeWidgetItem(tree_used_bcs);
  tree_used_bcs_displacements->setText(0,"Displacements");
  tree_used_bcs_displacements->setIcon(0,ccx_iface->getIcon2("HBCsDisplacementsTree"));
  tree_used_bcs_temperatures = new QTreeWidgetItem(tree_used_bcs);
  tree_used_bcs_temperatures->setText(0,"Temperatures");
  tree_used_bcs_temperatures->setIcon(0,ccx_iface->getIcon2("HBCsTemperaturesTree"));

  available_trees.push_back(tree_available_bcs_displacements);
  available_trees.push_back(tree_available_bcs_temperatures);

  used_trees.push_back(tree_used_bcs_displacements);
  used_trees.push_back(tree_used_bcs_temperatures);
  
  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_remove_clicked(bool)));
  QObject::connect(tree_hbcs, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(hbc_clicked(QTreeWidgetItem*, int)));
  QObject::connect(tree_hbcs, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,SLOT(hbc_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
  QObject::connect(tree_used, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(used_doubleclicked(QTreeWidgetItem*,int)));
  QObject::connect(tree_available, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(available_doubleclicked(QTreeWidgetItem*,int)));

  // add hbc item
  this->addHBC(QString::fromStdString("0"), QString::fromStdString("HBC"));
  // Update list items and data
  this->update();
}

HBCsManagement::~HBCsManagement()
{}

void HBCsManagement::clear()
{
  // Remove all of our tree items from hbcs tree.
  //this->removeItems();
  //current_hbc_item = NULL;
}

void HBCsManagement::update()
{
  this->setHBC(0);
}

void HBCsManagement::addHBC(QString hbc_id, QString hbc_name)
{
  QTreeWidgetItem* HBCsTreeChild = new QTreeWidgetItem(tree_hbcs);
  HBCsTreeChild->setText(1,hbc_id);
  HBCsTreeChild->setText(0,hbc_name);
  HBCsTreeChild->setIcon(0,ccx_iface->getIcon2("HBCsTree"));
}

void HBCsManagement::removeHBC(QTreeWidgetItem *hbc)
{
  tree_hbcs->removeItemWidget(hbc,0);
  delete hbc;
}

int HBCsManagement::get_child_id(std::string hbc_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;
  for (size_t i = 0; i < tree_hbcs->topLevelItemCount(); i++)
  {
    temp_child = tree_hbcs->topLevelItem(int(i));
    
    if (temp_child->text(1).toStdString()==hbc_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void HBCsManagement::createItems(QTreeWidgetItem *hbc)
{ 
  if (hbc==NULL)
  {
    return;
  }

  std::vector<std::vector<std::vector<std::string>>> available;
  std::vector<std::vector<std::vector<std::string>>> used;

  available.push_back(ccx_iface->get_bcsdisplacements_tree_data());
  available.push_back(ccx_iface->get_bcstemperatures_tree_data());
  
  used.push_back(ccx_iface->get_hbcsdisplacements_tree_data());
  used.push_back(ccx_iface->get_hbcstemperatures_tree_data());
  
  for (size_t i = 0; i < available.size(); i++)
  {
    for (size_t ii = 0; ii < available[i].size(); ii++)
    {
      QTreeWidgetItem* temp_child;
      temp_child = new QTreeWidgetItem(available_trees[i]);
      temp_child->setText(1,QString::fromStdString(available[i][ii][0]));
      temp_child->setText(0,QString::fromStdString(available[i][ii][1]));
      if (i==0)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("HBCsDisplacementsTree"));
      }else if (i==1)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("HBCsTemperaturesTree"));
      }
    }
    available_trees[i]->setExpanded(true);
  }
  tree_available_bcs->setExpanded(true);

  for (size_t i = 0; i < used.size(); i++)
  {
    for (size_t ii = 0; ii < used[i].size(); ii++)
    {
      QTreeWidgetItem* temp_child;
      temp_child = new QTreeWidgetItem(used_trees[i]);
      temp_child->setText(1,QString::fromStdString(used[i][ii][0]));
      temp_child->setText(0,QString::fromStdString(used[i][ii][1]));
      if (i==0)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("HBCsDisplacementsTree"));
      }else if (i==1)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("HBCsTemperaturesTree"));
      }
    }
    used_trees[i]->setExpanded(true);
  }
  tree_used_bcs->setExpanded(true);

  // remove used items from available items
  for (size_t i = 0; i < used.size(); i++)
  {
    for (size_t ii = 0; ii < used_trees[i]->childCount(); ii++)
    {
      for (size_t iii = 0; iii < available_trees[i]->childCount(); iii++)
      {
        if (used_trees[i]->child(int(ii))->text(1)==available_trees[i]->child(int(iii))->text(1))
        {
          available_trees[i]->removeChild(available_trees[i]->child(int(iii)));
        }
      }
    }
    //sort items
    used_trees[i]->sortChildren(1,Qt::AscendingOrder);
    available_trees[i]->sortChildren(1,Qt::AscendingOrder);
  }  
}

void HBCsManagement::removeItems()
{
  for (size_t i = 0; i < available_trees.size(); i++)
  {
    // Remove all of our tree items.
    while (available_trees[i]->childCount()>0)
    {
      available_trees[i]->removeChild(available_trees[i]->child(0));
    }
  }
  for (size_t i = 0; i < used_trees.size(); i++)
  {
    // Remove all of our tree items.
    while (used_trees[i]->childCount()>0)
    {
      used_trees[i]->removeChild(used_trees[i]->child(0));
    }
  }
}

void HBCsManagement::switchItem(QTreeWidgetItem* currentItem, QTreeWidgetItem* source, QTreeWidgetItem* target)
{
  if(currentItem!=NULL){
    QTreeWidgetItem* newItem = currentItem->clone();
    target->addChild(newItem);    
    delete currentItem;
    //this->selectItem(newItem);
    source->sortChildren(1,Qt::AscendingOrder);
    target->sortChildren(1,Qt::AscendingOrder);
  } 
}

void HBCsManagement::setHBC(QString hbc_id)
{
  QTreeWidgetItem* current_item;
  int ChildId;
  ChildId = get_child_id(hbc_id.toStdString());
  if (ChildId!=-1)
  {
    current_item = tree_hbcs->topLevelItem(ChildId);
    if (current_item!=NULL)
    {
      tree_hbcs->setCurrentItem(current_item);
    }
  }
}

void HBCsManagement::on_pushButton_ok_clicked(bool)
{
  //log = " clicked ok \n";
  //PRINT_INFO("%s", log.c_str());
  this->on_pushButton_apply_clicked(true);
  this->on_pushButton_close_clicked(true);
}

void HBCsManagement::on_pushButton_apply_clicked(bool)
{
  if (current_hbc_item==NULL)
  {
    return;
  }

  QStringList commands;
  QString command;
  QStringList command_prefix;
  bool found = false;

  std::vector<std::vector<std::vector<std::string>>> used;
 
  used.push_back(ccx_iface->get_hbcsdisplacements_tree_data());
  used.push_back(ccx_iface->get_hbcstemperatures_tree_data());
 
  command_prefix.push_back("ccx hbc add bc displacement ");
  command_prefix.push_back("ccx hbc add bc temperature ");

  command_prefix.push_back("ccx hbc remove bc displacement ");
  command_prefix.push_back("ccx hbc remove bc temperature ");

  for (size_t i = 0; i < used_trees.size(); i++)
  {
    for (size_t ii = 0; ii < used_trees[i]->childCount(); ii++)
    {
      found = false;
      for (size_t iii = 0; iii < used[i].size(); iii++)
      {
        if (used_trees[i]->child(int(ii))->text(1)==QString::fromStdString(used[i][iii][0]))
        {
          found = true;
        }
      }
      if (!found)
      {
        command = command_prefix[int(i)] + used_trees[i]->child(int(ii))->text(1);
        commands.push_back(command);
      }
    }
  } 

  for (size_t i = 0; i < used.size(); i++)
  {
    for (size_t ii = 0; ii < used[i].size(); ii++)
    {
      found = false;
      for (size_t iii = 0; iii < used_trees[i]->childCount(); iii++)
      {
        if (QString::fromStdString(used[i][ii][0])==used_trees[i]->child(int(iii))->text(1))
        {
          found = true;
        }
      }
      if (!found)
      {
        command = command_prefix[int(i)+2] + QString::fromStdString(used[i][ii][0]);
        commands.push_back(command);
      }
    }
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }

  this->update();
}

void HBCsManagement::on_pushButton_close_clicked(bool)
{
  //log = " clicked close \n";
  //PRINT_INFO("%s", log.c_str());
  this->close();
}

void HBCsManagement::on_pushButton_add_clicked(bool)
{
  QTreeWidgetItem* currentItem;  
  currentItem = tree_available->currentItem();
  if (currentItem != NULL)
  {
    for (size_t i = 0; i < available_trees.size(); i++)
    {
      if (currentItem->parent()==available_trees[i])
      {
        switchItem(currentItem, available_trees[i], used_trees[i]);
      }
    }
  }
}

void HBCsManagement::on_pushButton_remove_clicked(bool)
{
  QTreeWidgetItem* currentItem;  
  if (currentItem = tree_used->currentItem())
  {
    for (size_t i = 0; i < used_trees.size(); i++)
    {
      if (currentItem->parent()==used_trees[i])
      {
        switchItem(currentItem, used_trees[i], available_trees[i]);
      }
    }
  }
}

void HBCsManagement::hbc_clicked(QTreeWidgetItem* item, int column)
{
  current_hbc_item = item;
  if (item!=NULL)
  {    
    this->removeItems();
    this->createItems(item);
  }
}

void HBCsManagement::hbc_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item)
{
  if (current_item!=NULL)
  {
    this->hbc_clicked(current_item,0);
  }
}

void HBCsManagement::used_doubleclicked(QTreeWidgetItem* item, int column)
{
  //log = " doubleclicked used \n";
  //PRINT_INFO("%s", log.c_str());
  on_pushButton_remove_clicked(true);
}

void HBCsManagement::available_doubleclicked(QTreeWidgetItem* item, int column)
{
  //log = " doubleclicked available \n";
  //PRINT_INFO("%s", log.c_str());
  on_pushButton_add_clicked(true);
}