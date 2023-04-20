#include "MaterialManagement.hpp"
#include "CalculiXCoreInterface.hpp"
#include "MaterialManagementItem.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"

MaterialManagement::MaterialManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();


  // main window
  this->setGeometry(0,0,590,570);
  this->setWindowTitle("Material Management");
  
  // buttons
  pushButton_ok = new QPushButton(this);
  pushButton_ok->setGeometry(310,530,75,23);
  pushButton_ok->setText("OK");

  pushButton_apply = new QPushButton(this);
  pushButton_apply->setGeometry(400,530,75,23);
  pushButton_apply->setText("Apply");

  pushButton_close = new QPushButton(this);
  pushButton_close->setGeometry(490,530,75,23);
  pushButton_close->setText("Close");
  
  pushButton_new = new QPushButton(this);
  pushButton_new->setGeometry(200,30,75,23);
  pushButton_new->setText("New");

  pushButton_delete = new QPushButton(this);
  pushButton_delete->setGeometry(200,60,75,23);
  pushButton_delete->setText("Delete");
  
  pushButton_add = new QPushButton(this);
  pushButton_add->setGeometry(420,70,21,21);
  pushButton_add->setText(">");

  pushButton_remove = new QPushButton(this);
  pushButton_remove->setGeometry(420,100,21,21);
  pushButton_remove->setText("<");

  // labels
  label_material = new QLabel(this);
  label_material->setGeometry(10,10,80,16);
  label_material->setText("Material");
  
  label_available = new QLabel(this);
  label_available->setGeometry(450,10,100,16);
  label_available->setText("Available Cards");

  label_used = new QLabel(this);
  label_used->setGeometry(300,10,100,16);
  label_used->setText("Used Cards");

  // tree/lists
  tree_material = new QTreeWidget(this);
  tree_material->setGeometry(10,30,181,191);
  tree_material->setColumnCount(2);
  tree_material->setHeaderLabels(QStringList() << "Name" << "ID");

  list_available = new QListWidget(this);
  list_available->setGeometry(450,30,111,191);

  list_used = new QListWidget(this);
  list_used->setGeometry(300,30,111,191);

  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_new, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_new_clicked(bool)));
  QObject::connect(pushButton_delete, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_delete_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_remove_clicked(bool)));

  // Update list items and data
  this->update();
}

MaterialManagement::~MaterialManagement()
{}

void MaterialManagement::clear()
{
  // Remove all of our tree items from material tree.
  while (tree_material->topLevelItemCount()>0)
  {
    tree_material->removeItemWidget(tree_material->topLevelItem(0),0);
  }
}

void MaterialManagement::update()
{
  std::vector<std::vector<std::string>> material_tree_data;
  material_tree_data = ccx_iface->get_material_tree_data(); 

  MaterialManagementItem *temp_child;
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
      temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(ChildId));
      
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
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i-1));

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
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i-1));
    temp_child->update();
  }
}

void MaterialManagement::addMaterial(QString material_id, QString material_name)
{
  MaterialManagementItem *MaterialTreeChild = new MaterialManagementItem(tree_material);
  MaterialTreeChild->initialize(material_id, material_name);
}

void MaterialManagement::removeMaterial(MaterialManagementItem *material)
{
  tree_material->removeItemWidget(material,0);
  delete material;
}

void MaterialManagement::printproperties()
{
  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i));
    
    log = "########### \n";
    log.append(temp_child->text(1).toStdString() + " child id \n");
    for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
    {
      log.append("----------\n");
      log.append(temp_child->group_properties[temp_child->properties[ii][0]][0] + " \n");
      log.append("----------\n");
      log.append(std::to_string(temp_child->properties[ii][0]) + " " + std::to_string(temp_child->properties[ii][1]) + " " + std::to_string(temp_child->properties[ii][2]) + " \n");
      if (temp_child->properties[ii][1]==1)
      {
        log.append("SCALAR\n");
        log.append(std::to_string(temp_child->property_scalar[temp_child->properties[ii][2]]) + " \n");
      } else if (temp_child->properties[ii][1]==4)
      {        
        log.append("MATRIX\n");
        for (size_t iii = 0; iii < temp_child->property_matrix[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
      }
    }
    PRINT_INFO("%s", log.c_str());
  }
}

int MaterialManagement::get_child_id(std::string material_id)
{
  int int_return;
  int_return = -1;

  //log = std::to_string(tree_material->topLevelItemCount()) + " child count \n";
  //PRINT_INFO("%s", log.c_str());

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i));
    
    //log = temp_child->text(1).toStdString() + " == " + material_id + " child ids \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->text(1).toStdString()==material_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}

void MaterialManagement::on_pushButton_ok_clicked(bool)
{
  log = " clicked ok \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_apply_clicked(bool)
{
  QStringList commands;

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
}

void MaterialManagement::on_pushButton_close_clicked(bool)
{
  //log = " clicked close \n";
  //PRINT_INFO("%s", log.c_str());
  this->close();
}

void MaterialManagement::on_pushButton_new_clicked(bool)
{
  log = " clicked new \n";
  PRINT_INFO("%s", log.c_str());

  this->printproperties();
}

void MaterialManagement::on_pushButton_delete_clicked(bool)
{
  log = " clicked delete \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_add_clicked(bool)
{
  log = " clicked add \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_remove_clicked(bool)
{
  log = " clicked remove \n";
  PRINT_INFO("%s", log.c_str());
}