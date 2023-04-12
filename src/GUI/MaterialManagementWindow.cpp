#include "MaterialManagementWindow.hpp"
#include "ui_MaterialManagementWindow.h"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"
#include "PickWidget.hpp"


MaterialManagement::MaterialManagement(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MaterialManagement)
{
  ui->setupUi(this);
}

MaterialManagement::~MaterialManagement()
{
  delete ui;
}

void MaterialManagement::on_pbApply_clicked()
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
