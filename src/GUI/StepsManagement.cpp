#include "StepsManagement.hpp"
#include "CalculiXCoreInterface.hpp"
#include "StepsManagementItem.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"

StepsManagement::StepsManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
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
  
  /*
  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_remove_clicked(bool)));
  */
  // Update list items and data
  this->update();
}

StepsManagement::~StepsManagement()
{}

void StepsManagement::clear()
{
  /*
  // Remove all of our tree items from material tree.
  while (tree_material->topLevelItemCount()>0)
  {
    tree_material->removeItemWidget(tree_material->topLevelItem(0),0);
  }
  */
}

void StepsManagement::update()
{
  /*
  std::vector<std::vector<std::string>> material_tree_data;
  material_tree_data = ccx_iface->get_material_tree_data(); 

  StepsManagementItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < material_tree_data.size(); i++)
  {
    // check if material already exists as item
    
    ChildId = this->get_child_id(material_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addMaterial(QString::fromStdString(material_tree_data[i][0]), QString::fromStdString(material_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = dynamic_cast<StepsManagementItem*>(tree_material->topLevelItem(ChildId));
      
      str_check = material_tree_data[i][1];    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }

  // check if item has been removed

  for (size_t i = tree_material->topLevelItemCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = dynamic_cast<StepsManagementItem*>(tree_material->topLevelItem(i-1));

    for (size_t ii = 0; ii < material_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==material_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      this->removeMaterial(temp_child);
    }
  }

  // update childs
  for (size_t i = tree_material->topLevelItemCount(); i > 0; i--)
  { 
    temp_child = dynamic_cast<StepsManagementItem*>(tree_material->topLevelItem(i-1));
    temp_child->update();
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

  StepsManagementItem *temp_child;
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
