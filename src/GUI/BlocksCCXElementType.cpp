#include "BlocksCCXElementType.hpp"
#include "ui_BlocksCCXElementType.h"

#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"
#include "PickWidget.hpp"


BlocksCCXElementType::BlocksCCXElementType(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::BlocksCCXElementType)
{
  ui->setupUi(this);

  // Create the pickwidget and set it's pick type
  pwBlockIds = new PickWidget();
  pwBlockIds->setPickType(PickWidget::Block);

  // Add the pickwidget to our pickwidget container in the ui file. Note
  // that our container needs a Qt Layout first, so we will create one
  // and add our pickwidget to the layout.
  QGridLayout* pickwidget_layout = new QGridLayout(ui->pwBlock);
  pickwidget_layout->addWidget(pwBlockIds);
}

BlocksCCXElementType::~BlocksCCXElementType()
{
  delete ui;

  if(pwBlockIds)
    delete pwBlockIds;
}

void BlocksCCXElementType::on_pbApply_clicked()
{
  QStringList commands;

  commands.push_back("reset");

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
