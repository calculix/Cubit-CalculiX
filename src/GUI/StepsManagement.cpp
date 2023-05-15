#include "StepsManagement.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"

StepsManagement::StepsManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  this->setGeometry(0,0,700,570);
  this->setWindowTitle("Steps Management");

  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_steps_1 = new QVBoxLayout();
  boxLayout_assignment_1 = new QVBoxLayout();
  boxLayout_assignment_2 = new QVBoxLayout();
  boxLayout_assignment_3 = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,1,3, Qt::AlignRight);
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
  tree_steps->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_steps_1->addWidget(tree_steps);

  tree_available = new QTreeWidget();
  tree_available->setColumnCount(2);
  tree_available->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_assignment_1->addWidget(tree_available);

  tree_used = new QTreeWidget();
  tree_used->setColumnCount(2);
  tree_used->setHeaderLabels(QStringList() << "Name" << "ID");
  //list_available->setGeometry(300,30,111,191);
  boxLayout_assignment_3->addWidget(tree_used);


  tree_available_loads = new QTreeWidgetItem(tree_available);
  tree_available_loads->setText(0,"Loads");
  tree_available_loads_forces = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_forces->setText(0,"Forces");
  tree_available_loads_pressures = new QTreeWidgetItem(tree_available_loads);
  tree_available_loads_pressures->setText(0,"Pressures");
  tree_available_bcs = new QTreeWidgetItem(tree_available);
  tree_available_bcs->setText(0,"BCs");
  tree_available_bcs_displacements = new QTreeWidgetItem(tree_available_bcs);
  tree_available_bcs_displacements->setText(0,"Displacements");
  tree_available_bcs_temperatures = new QTreeWidgetItem(tree_available_bcs);
  tree_available_bcs_temperatures->setText(0,"Temperatures");
  tree_available_historyoutputs = new QTreeWidgetItem(tree_available);
  tree_available_historyoutputs->setText(0,"History Outputs");
  tree_available_fieldoutputs = new QTreeWidgetItem(tree_available);
  tree_available_fieldoutputs->setText(0,"Field Outputs");

  tree_used_loads = new QTreeWidgetItem(tree_used);
  tree_used_loads->setText(0,"Loads");
  tree_used_loads_forces = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_forces->setText(0,"Forces");
  tree_used_loads_pressures = new QTreeWidgetItem(tree_used_loads);
  tree_used_loads_pressures->setText(0,"Pressures");
  tree_used_bcs = new QTreeWidgetItem(tree_used);
  tree_used_bcs->setText(0,"BCs");
  tree_used_bcs_displacements = new QTreeWidgetItem(tree_used_bcs);
  tree_used_bcs_displacements->setText(0,"Displacements");
  tree_used_bcs_temperatures = new QTreeWidgetItem(tree_used_bcs);
  tree_used_bcs_temperatures->setText(0,"Temperatures");
  tree_used_historyoutputs = new QTreeWidgetItem(tree_used);
  tree_used_historyoutputs->setText(0,"History Outputs");
  tree_used_fieldoutputs = new QTreeWidgetItem(tree_used);
  tree_used_fieldoutputs->setText(0,"Field Outputs");

  available_trees.push_back(tree_available_loads_forces);
  available_trees.push_back(tree_available_loads_pressures);
  available_trees.push_back(tree_available_bcs_displacements);
  available_trees.push_back(tree_available_bcs_temperatures);
  available_trees.push_back(tree_available_historyoutputs);
  available_trees.push_back(tree_available_fieldoutputs);

  used_trees.push_back(tree_used_loads_forces);
  used_trees.push_back(tree_used_loads_pressures);
  used_trees.push_back(tree_used_bcs_displacements);
  used_trees.push_back(tree_used_bcs_temperatures);
  used_trees.push_back(tree_used_historyoutputs);
  used_trees.push_back(tree_used_fieldoutputs);
  
  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_remove_clicked(bool)));
  QObject::connect(tree_steps, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,  SLOT(step_clicked(QTreeWidgetItem*, int)));
  QObject::connect(tree_steps, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,  SLOT(step_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
  
  // Update list items and data
  this->update();
}

StepsManagement::~StepsManagement()
{}

void StepsManagement::clear()
{
  // Remove all of our tree items from steps tree.
  while (tree_steps->topLevelItemCount()>0)
  {
    tree_steps->removeItemWidget(tree_steps->topLevelItem(0),0);
  }
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
    temp_child = tree_steps->topLevelItem(i-1);
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
    temp_child = tree_steps->topLevelItem(i);
    //log = temp_child->text(1).toStdString() + " == " + step_id + " child ids \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->text(1).toStdString()==step_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}

void StepsManagement::createItems(QTreeWidgetItem *step)
{  
  std::vector<std::vector<std::vector<std::string>>> available;
  std::vector<std::vector<std::vector<std::string>>> used;

  available.push_back(ccx_iface->get_loadsforces_tree_data());
  available.push_back(ccx_iface->get_loadspressures_tree_data());
  available.push_back(ccx_iface->get_bcsdisplacements_tree_data());
  available.push_back(ccx_iface->get_bcstemperatures_tree_data());
  available.push_back(ccx_iface->get_historyoutputs_tree_data());
  available.push_back(ccx_iface->get_fieldoutputs_tree_data());

  used.push_back(ccx_iface->get_steps_loadsforces_tree_data(step->text(1).toInt()));
  used.push_back(ccx_iface->get_steps_loadspressures_tree_data(step->text(1).toInt()));
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
        if (used_trees[i]->child(ii)->text(1)==available_trees[i]->child(iii)->text(1))
        {
          available_trees[i]->removeChild(available_trees[i]->child(iii));
        }
      }
    }
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

void StepsManagement::switchItem(QTreeWidgetItem* source, QTreeWidgetItem* target)
{
  /*
  if(source->currentItem()){
    QListWidgetItem* newItem = source->currentItem()->clone();
    target->addItem(newItem);
    target->setCurrentItem(newItem);
    delete source->currentItem();
    this->selectListItem(newItem);
    this->loadWidget(newItem);
    source->sortItems();
    target->sortItems();

    if (newItem->text()=="Elastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_ELASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_ELASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_ELASTIC_USE_CARD", 0);
      }
    }else if (newItem->text()=="Plastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_PLASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_PLASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_PLASTIC_USE_CARD", 0);
      }
    }else if (newItem->text()=="Density")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_DENSITY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_DENSITY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_DENSITY_USE_CARD", 0);
      }
    }else if (newItem->text()=="Specific heat")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_SPECIFIC_HEAT_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_SPECIFIC_HEAT_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_SPECIFIC_HEAT_USE_CARD", 0);
      }
    }else if (newItem->text()=="Expansion")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_EXPANSION_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_EXPANSION_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_EXPANSION_USE_CARD", 0);
      }
    }else if (newItem->text()=="Conductivity")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_CONDUCTIVITY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_CONDUCTIVITY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_CONDUCTIVITY_USE_CARD", 0);
      } 
    }
  }
  */
}

void StepsManagement::selectItem(QTreeWidgetItem* item)
{
  /*
  QTreeWidgetItem* temp_item;

  for (size_t i = 0; i < list_available->count(); i++)
  {
    temp_item = list_available->item(i);
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }
  for (size_t i = 0; i < list_used->count(); i++)
  {
    temp_item = list_used->item(i);
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }
  */ 
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
  QStringList commands;
  //this->printproperties();
  QString command;
  QString command_prefix;

  QTreeWidgetItem *temp_child;
/*
  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<StepsManagementItem*>(tree_material->topLevelItem(i));
    command_prefix = "modify material \"" + temp_child->material_name_qstring + "\" ";

    for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
    {
      command = "";

      if ((temp_child->properties[ii][1]==1) &&
          (temp_child->property_scalar[temp_child->properties[ii][2]] != temp_child->property_scalar_gui[temp_child->properties[ii][2]]))
      {
        command.append(command_prefix);
        // scalar
        command.append("scalar_properties ");
        // group property
        command.append("\"" + QString::fromStdString(temp_child->group_properties[temp_child->properties[ii][0]][0]) + "\" ");
        // value
        command.append(QString::number(temp_child->property_scalar_gui[temp_child->properties[ii][2]]));
      } else if ((temp_child->properties[ii][1]==4) &&
          (temp_child->property_matrix[temp_child->properties[ii][2]] != temp_child->property_matrix_gui[temp_child->properties[ii][2]]))
      { 
        command.append(command_prefix);
        // matrix
        command.append("matrix_property ");
        // group property
        command.append("\"" + QString::fromStdString(temp_child->group_properties[temp_child->properties[ii][0]][0]) + "\" ");
        // value
        for (size_t iii = 0; iii < temp_child->property_matrix_gui[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            command.append(QString::number(temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
        }
      }
      if (command != "")
      {
        commands.push_back(command);
      }
    }
    
  }

  // We must send the Cubit commands through the Claro framework, so first we need to translate
  // the commands into the python form that Claro will understand.
  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    for(int i = 0; i < commands.size(); i++)
      cubit_translator->decode(commands[i]);

    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
  */
}

void StepsManagement::on_pushButton_close_clicked(bool)
{
  //log = " clicked close \n";
  //PRINT_INFO("%s", log.c_str());
  this->close();
}

void StepsManagement::on_pushButton_add_clicked(bool)
{
  //log = " clicked add \n";
  //PRINT_INFO("%s", log.c_str());
  //switchListItem(list_available, list_used);
}

void StepsManagement::on_pushButton_remove_clicked(bool)
{
  //log = " clicked remove \n";
  //PRINT_INFO("%s", log.c_str());
  //switchListItem(list_used, list_available);
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
  if (current_item!=nullptr)
  {
    this->step_clicked(current_item,0);
  }
}