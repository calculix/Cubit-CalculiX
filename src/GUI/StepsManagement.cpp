#include "StepsManagement.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"

StepsManagement::StepsManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  this->setGeometry(0,0,1100,800);
  this->setWindowTitle("Steps Management");

  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_steps_1 = new QVBoxLayout();
  boxLayout_assignment_1 = new QVBoxLayout();
  boxLayout_assignment_2 = new QVBoxLayout();
  boxLayout_assignment_3 = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,1,2,1,2, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_steps_1,0,0, Qt::AlignTop);
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
  label_steps = new QLabel();
  //label_material->setGeometry(10,10,80,16);
  label_steps->setText("Steps");
  boxLayout_steps_1->addWidget(label_steps);

  label_available = new QLabel();
  //label_available->setGeometry(300,10,100,16);
  label_available->setText("Available");
  boxLayout_assignment_1->addWidget(label_available);

  label_used = new QLabel();
  //label_used->setGeometry(450,10,100,16);
  label_used->setText("Used");
  boxLayout_assignment_3->addWidget(label_used);

  // tree/lists
  tree_steps = new QTreeWidget();
  //tree_material->setGeometry(10,30,181,191);
  tree_steps->setColumnCount(2);
  tree_steps->setColumnWidth(0, 200);
  tree_steps->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_steps_1->addWidget(tree_steps);

  tree_available = new QTreeWidget();
  tree_available->setColumnCount(2);
  tree_available->setColumnWidth(0, 200);
  tree_available->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_assignment_1->addWidget(tree_available);

  tree_used = new QTreeWidget();
  tree_used->setColumnCount(2);
  tree_used->setColumnWidth(0, 200);
  tree_used->setHeaderLabels(QStringList() << "Name" << "ID");
  //list_available->setGeometry(300,30,111,191);
  boxLayout_assignment_3->addWidget(tree_used);


  tree_available_loads = new QTreeWidgetItem(tree_available);
  tree_available_loads->setText(0,"Loads");
  tree_available_loads->setIcon(0,ccx_iface->getIcon2("StepsLoadsTree"));
  tree_available_loads_forces = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_forces->setText(0,"Forces");
  tree_available_loads_forces->setIcon(0,ccx_iface->getIcon2("StepsLoadsForcesTree"));
  tree_available_loads_pressures = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_pressures->setText(0,"Pressures");
  tree_available_loads_pressures->setIcon(0,ccx_iface->getIcon2("StepsLoadsPressuresTree"));
  tree_available_loads_heatfluxes = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_heatfluxes->setText(0,"Heatfluxes");
  tree_available_loads_heatfluxes->setIcon(0,ccx_iface->getIcon2("StepsLoadsHeatfluxesTree"));
  tree_available_loads_gravity = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_gravity->setText(0,"Gravity");
  tree_available_loads_gravity->setIcon(0,ccx_iface->getIcon2("StepsLoadsGravityTree"));
  tree_available_loads_centrifugal = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_centrifugal->setText(0,"Centrifugal");
  tree_available_loads_centrifugal->setIcon(0,ccx_iface->getIcon2("StepsLoadsCentrifugalTree"));
  tree_available_loads_trajectory = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_trajectory->setText(0,"Trajectory");
  tree_available_loads_trajectory->setIcon(0,ccx_iface->getIcon2("StepsLoadsTrajectoryTree"));
  tree_available_loads_film = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_film->setText(0,"Film");
  tree_available_loads_film->setIcon(0,ccx_iface->getIcon2("StepsLoadsFilmTree"));
  tree_available_loads_radiation = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_radiation->setText(0,"Radiation");
  tree_available_loads_radiation->setIcon(0,ccx_iface->getIcon2("StepsLoadsRadiationTree"));
  tree_available_bcs = new QTreeWidgetItem(tree_available);
  tree_available_bcs->setText(0,"BCs");
  tree_available_bcs->setIcon(0,ccx_iface->getIcon2("StepsBCsTree"));
  tree_available_bcs_displacements = new QTreeWidgetItem(tree_available_bcs);
  tree_available_bcs_displacements->setText(0,"Displacements");
  tree_available_bcs_displacements->setIcon(0,ccx_iface->getIcon2("StepsBCsDisplacementsTree"));
  tree_available_bcs_temperatures = new QTreeWidgetItem(tree_available_bcs);
  tree_available_bcs_temperatures->setText(0,"Temperatures");
  tree_available_bcs_temperatures->setIcon(0,ccx_iface->getIcon2("StepsBCsTemperaturesTree"));
  tree_available_historyoutputs = new QTreeWidgetItem(tree_available);
  tree_available_historyoutputs->setText(0,"History Outputs");
  tree_available_historyoutputs->setIcon(0,ccx_iface->getIcon2("StepsHistoryOutputsTree"));
  tree_available_fieldoutputs = new QTreeWidgetItem(tree_available);
  tree_available_fieldoutputs->setText(0,"Field Outputs");
  tree_available_fieldoutputs->setIcon(0,ccx_iface->getIcon2("StepsFieldOutputsTree"));

  tree_used_loads = new QTreeWidgetItem(tree_used);
  tree_used_loads->setText(0,"Loads");
  tree_used_loads->setIcon(0,ccx_iface->getIcon2("StepsLoadsTree"));
  tree_used_loads_forces = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_forces->setText(0,"Forces");
  tree_used_loads_forces->setIcon(0,ccx_iface->getIcon2("StepsLoadsForcesTree"));
  tree_used_loads_pressures = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_pressures->setText(0,"Pressures");
  tree_used_loads_pressures->setIcon(0,ccx_iface->getIcon2("StepsLoadsPressuresTree"));
  tree_used_loads_heatfluxes = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_heatfluxes->setText(0,"Heatfluxes");
  tree_used_loads_heatfluxes->setIcon(0,ccx_iface->getIcon2("StepsLoadsHeatfluxesTree"));
  tree_used_loads_gravity = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_gravity->setText(0,"Gravity");
  tree_used_loads_gravity->setIcon(0,ccx_iface->getIcon2("StepsLoadsGravityTree"));
  tree_used_loads_centrifugal = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_centrifugal->setText(0,"Centrifugal");
  tree_used_loads_centrifugal->setIcon(0,ccx_iface->getIcon2("StepsLoadsCentrifugalTree"));
  tree_used_loads_trajectory = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_trajectory->setText(0,"Trajectory");
  tree_used_loads_trajectory->setIcon(0,ccx_iface->getIcon2("StepsLoadsTrajectoryTree"));
  tree_used_loads_film = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_film->setText(0,"Film");
  tree_used_loads_film->setIcon(0,ccx_iface->getIcon2("StepsLoadsFilmTree"));
  tree_used_loads_radiation = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_radiation->setText(0,"Radiation");
  tree_used_loads_radiation->setIcon(0,ccx_iface->getIcon2("StepsLoadsRadiationTree"));
  tree_used_bcs = new QTreeWidgetItem(tree_used);
  tree_used_bcs->setText(0,"BCs");
  tree_used_bcs->setIcon(0,ccx_iface->getIcon2("StepsBCsTree"));
  tree_used_bcs_displacements = new QTreeWidgetItem(tree_used_bcs);
  tree_used_bcs_displacements->setText(0,"Displacements");
  tree_used_bcs_displacements->setIcon(0,ccx_iface->getIcon2("StepsBCsDisplacementsTree"));
  tree_used_bcs_temperatures = new QTreeWidgetItem(tree_used_bcs);
  tree_used_bcs_temperatures->setText(0,"Temperatures");
  tree_used_bcs_temperatures->setIcon(0,ccx_iface->getIcon2("StepsBCsTemperaturesTree"));
  tree_used_historyoutputs = new QTreeWidgetItem(tree_used);
  tree_used_historyoutputs->setText(0,"History Outputs");
  tree_used_historyoutputs->setIcon(0,ccx_iface->getIcon2("StepsHistoryOutputsTree"));
  tree_used_fieldoutputs = new QTreeWidgetItem(tree_used);
  tree_used_fieldoutputs->setText(0,"Field Outputs");
  tree_used_fieldoutputs->setIcon(0,ccx_iface->getIcon2("StepsFieldOutputsTree"));

  available_trees.push_back(tree_available_loads_forces);
  available_trees.push_back(tree_available_loads_pressures);
  available_trees.push_back(tree_available_loads_heatfluxes);
  available_trees.push_back(tree_available_loads_gravity);
  available_trees.push_back(tree_available_loads_centrifugal);
  available_trees.push_back(tree_available_loads_trajectory);
  available_trees.push_back(tree_available_loads_film);
  available_trees.push_back(tree_available_loads_radiation);
  available_trees.push_back(tree_available_bcs_displacements);
  available_trees.push_back(tree_available_bcs_temperatures);
  available_trees.push_back(tree_available_historyoutputs);
  available_trees.push_back(tree_available_fieldoutputs);

  used_trees.push_back(tree_used_loads_forces);
  used_trees.push_back(tree_used_loads_pressures);
  used_trees.push_back(tree_used_loads_heatfluxes);
  used_trees.push_back(tree_used_loads_gravity);
  used_trees.push_back(tree_used_loads_centrifugal);
  used_trees.push_back(tree_used_loads_trajectory);
  used_trees.push_back(tree_used_loads_film);
  used_trees.push_back(tree_used_loads_radiation);
  used_trees.push_back(tree_used_bcs_displacements);
  used_trees.push_back(tree_used_bcs_temperatures);
  used_trees.push_back(tree_used_historyoutputs);
  used_trees.push_back(tree_used_fieldoutputs);
  
  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_remove_clicked(bool)));
  QObject::connect(tree_steps, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(step_clicked(QTreeWidgetItem*, int)));
  QObject::connect(tree_steps, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,SLOT(step_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
  QObject::connect(tree_used, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(used_doubleclicked(QTreeWidgetItem*,int)));
  QObject::connect(tree_available, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(available_doubleclicked(QTreeWidgetItem*,int)));

  // Update list items and data
  this->update();
}

StepsManagement::~StepsManagement()
{}

void StepsManagement::clear()
{
  // Remove all of our tree items from steps tree.
  /*
  while (tree_steps->topLevelItemCount()>0)
  {
    tree_steps->removeItemWidget(tree_steps->topLevelItem(0),0);
  }
  */
  this->removeItems();
  current_step_item = NULL;
}

void StepsManagement::update()
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
      this->addStep(QString::fromStdString(steps_tree_data[i][0]), QString::fromStdString(steps_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = tree_steps->topLevelItem(ChildId);
      str_check = steps_tree_data[i][1];    
      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }
  
  // check if item has been removed
  for (size_t i = tree_steps->topLevelItemCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = tree_steps->topLevelItem(int(i)-1);
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
      this->removeStep(temp_child);
    }
  }
}

void StepsManagement::addStep(QString step_id, QString step_name)
{
  QTreeWidgetItem* StepsTreeChild = new QTreeWidgetItem(tree_steps);
  StepsTreeChild->setText(1,step_id);
  StepsTreeChild->setText(0,step_name);
  StepsTreeChild->setIcon(0,ccx_iface->getIcon2("StepsTree"));
}

void StepsManagement::removeStep(QTreeWidgetItem *step)
{
  tree_steps->removeItemWidget(step,0);
  delete step;
}

int StepsManagement::get_child_id(std::string step_id)
{
  int int_return;
  int_return = -1;

  QTreeWidgetItem *temp_child;
  for (size_t i = 0; i < tree_steps->topLevelItemCount(); i++)
  {
    temp_child = tree_steps->topLevelItem(int(i));
    //log = temp_child->text(1).toStdString() + " == " + step_id + " child ids \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->text(1).toStdString()==step_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

void StepsManagement::createItems(QTreeWidgetItem *step)
{ 
  if (step==NULL)
  {
    return;
  }

  std::vector<std::vector<std::vector<std::string>>> available;
  std::vector<std::vector<std::vector<std::string>>> used;

  available.push_back(ccx_iface->get_loadsforces_tree_data());
  available.push_back(ccx_iface->get_loadspressures_tree_data());
  available.push_back(ccx_iface->get_loadsheatfluxes_tree_data());
  available.push_back(ccx_iface->get_loadsgravity_tree_data());
  available.push_back(ccx_iface->get_loadscentrifugal_tree_data());
  available.push_back(ccx_iface->get_loadstrajectory_tree_data());
  available.push_back(ccx_iface->get_loadsfilm_tree_data());
  available.push_back(ccx_iface->get_loadsradiation_tree_data());
  available.push_back(ccx_iface->get_bcsdisplacements_tree_data());
  available.push_back(ccx_iface->get_bcstemperatures_tree_data());
  available.push_back(ccx_iface->get_historyoutputs_tree_data());
  available.push_back(ccx_iface->get_fieldoutputs_tree_data());

  used.push_back(ccx_iface->get_steps_loadsforces_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadspressures_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsheatfluxes_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsgravity_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadscentrifugal_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadstrajectory_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsfilm_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsradiation_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_bcsdisplacements_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_bcstemperatures_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_historyoutputs_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_fieldoutputs_tree_data(step->text(1).toInt()));

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
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsForcesTree"));
      }else if (i==1)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsPressuresTree"));
      }else if (i==2)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsHeatfluxesTree"));
      }else if (i==3)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsGravityTree"));
      }else if (i==4)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsCentrifugalTree"));
      }else if (i==5)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsTrajectoryTree"));
      }else if (i==6)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsFilmTree"));
      }else if (i==7)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsRadiationTree"));
      }else if (i==8)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsBCsDisplacementsTree"));
      }else if (i==9)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsBCsTemperaturesTree"));
      }else if (i==10)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsHistoryOutputsTree"));
      }else if (i==11)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsFieldOutputsTree"));
      }
    }
    available_trees[i]->setExpanded(true);
  }
  tree_available_loads->setExpanded(true);
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
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsForcesTree"));
      }else if (i==1)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsPressuresTree"));
      }else if (i==2)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsHeatfluxesTree"));
      }else if (i==3)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsGravityTree"));
      }else if (i==4)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsCentrifugalTree"));
      }else if (i==5)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsTrajectoryTree"));
      }else if (i==6)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsFilmTree"));
      }else if (i==7)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsLoadsRadiationTree"));
      }else if (i==8)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsBCsDisplacementsTree"));
      }else if (i==9)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsBCsTemperaturesTree"));
      }else if (i==10)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsHistoryOutputsTree"));
      }else if (i==11)
      {
        temp_child->setIcon(0,ccx_iface->getIcon2("StepsFieldOutputsTree"));
      }
    }
    used_trees[i]->setExpanded(true);
  }
  tree_used_loads->setExpanded(true);
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

void StepsManagement::removeItems()
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

void StepsManagement::switchItem(QTreeWidgetItem* currentItem, QTreeWidgetItem* source, QTreeWidgetItem* target)
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

void StepsManagement::setStep(QString step_id)
{
  QTreeWidgetItem* current_item;
  int ChildId;
  ChildId = get_child_id(step_id.toStdString());
  if (ChildId!=-1)
  {
    current_item = tree_steps->topLevelItem(ChildId);
    if (current_item!=NULL)
    {
      tree_steps->setCurrentItem(current_item);
    }
  }
}

void StepsManagement::on_pushButton_ok_clicked(bool)
{
  //log = " clicked ok \n";
  //PRINT_INFO("%s", log.c_str());
  this->on_pushButton_apply_clicked(true);
  this->on_pushButton_close_clicked(true);
}

void StepsManagement::on_pushButton_apply_clicked(bool)
{
  if (current_step_item==NULL)
  {
    return;
  }
  
  QStringList commands;
  QString command;
  QStringList command_prefix;
  bool found = false;

  std::vector<std::vector<std::vector<std::string>>> used;
 
  used.push_back(ccx_iface->get_steps_loadsforces_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadspressures_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsheatfluxes_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsgravity_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadscentrifugal_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadstrajectory_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsfilm_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadsradiation_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_bcsdisplacements_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_bcstemperatures_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_historyoutputs_tree_data(current_step_item->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_fieldoutputs_tree_data(current_step_item->text(1).toInt()));

  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load force ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load pressure ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load heatflux ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load gravity ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load centrifugal ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load trajectory ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load film ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add load radiation ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add bc displacement ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add bc temperature ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add historyoutput ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " add fieldoutput ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load force ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load pressure ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load heatflux ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load gravity ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load centrifugal ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load trajectory ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load film ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove load radiation ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove bc displacement ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove bc temperature ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove historyoutput ");
  command_prefix.push_back("ccx step " + current_step_item->text(1) + " remove fieldoutput ");

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
      { // prefix shift for remove
        command = command_prefix[int(i)+9] + QString::fromStdString(used[i][ii][0]);
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

void StepsManagement::on_pushButton_close_clicked(bool)
{
  //log = " clicked close \n";
  //PRINT_INFO("%s", log.c_str());
  this->close();
}

void StepsManagement::on_pushButton_add_clicked(bool)
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

void StepsManagement::on_pushButton_remove_clicked(bool)
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

void StepsManagement::step_clicked(QTreeWidgetItem* item, int column)
{
  current_step_item = item;
  if (item!=NULL)
  {    
    this->removeItems();
    this->createItems(item);
  }
}

void StepsManagement::step_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item)
{
  if (current_item!=NULL)
  {
    this->step_clicked(current_item,0);
  }
}

void StepsManagement::used_doubleclicked(QTreeWidgetItem* item, int column)
{
  //log = " doubleclicked used \n";
  //PRINT_INFO("%s", log.c_str());
  on_pushButton_remove_clicked(true);
}

void StepsManagement::available_doubleclicked(QTreeWidgetItem* item, int column)
{
  //log = " doubleclicked available \n";
  //PRINT_INFO("%s", log.c_str());
  on_pushButton_add_clicked(true);
}