#include "BlocksCCXElementTypePanel.hpp"
#include "ui_BlocksCCXElementTypePanel.h"

#include "CubitInterface.hpp"
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

  //group and uncheck all radio buttons
  QList<QRadioButton *> allButtons = ui->groupBox->findChildren<QRadioButton *>();
  for (size_t i = 0; i < allButtons.size(); i++)
  {
    group.addButton(allButtons[i],i);
  }
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

  if (group.exclusive())
  {
    QRadioButton *rb = dynamic_cast<QRadioButton*>(group.checkedButton());
    if (rb)
    {
      //std::string pickwidget_text = pwBlockIds->text().toUtf8().data();
      commands.push_back("ccx block " + pwBlockIds->text() + " element_type "  + rb->text());
      /*
      std::vector<int> ids = CubitInterface::parse_cubit_list("block", pickwidget_text);
      for (size_t i = 0; i < ids.size(); i++)
      {
        commands.push_back("ccx block " + QString::number(ids[i]) + " element_type "  + rb->text());
      }
      */
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
}
