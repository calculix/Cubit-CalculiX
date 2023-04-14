#include "MaterialManagement.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"


MaterialManagement::MaterialManagement()
{
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

  // lists

  // Signals
}

MaterialManagement::~MaterialManagement()
{
}

void MaterialManagement::on_pushButton_ok_clicked()
{
  log = " clicked ok \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_apply_clicked()
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

void MaterialManagement::on_pushButton_close_clicked()
{
  log = " clicked close \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_new_clicked()
{
  log = " clicked new \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_delete_clicked()
{
  log = " clicked delete \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_add_clicked()
{
  log = " clicked add \n";
  PRINT_INFO("%s", log.c_str());
}

void MaterialManagement::on_pushButton_remove_clicked()
{
  log = " clicked remove \n";
  PRINT_INFO("%s", log.c_str());
}