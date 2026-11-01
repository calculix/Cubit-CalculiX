#include "BCsDisplacementsModifyPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


BCsDisplacementsModifyPanel::BCsDisplacementsModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();
  HBoxLayout_7 = new QHBoxLayout();
  HBoxLayout_8 = new QHBoxLayout();
  HBoxLayout_9 = new QHBoxLayout();
  HBoxLayout_10 = new QHBoxLayout();
  HBoxLayout_11 = new QHBoxLayout();
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_10 = new QLabel();
  label_11 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  label_10->setFixedWidth(labelWidth);
  label_11->setFixedWidth(labelWidth);
  label_0->setText("Displacement ID");
  label_1->setText("Name");
  label_2->setText("Nodeset ID");
  label_3->setText("Displacement X");
  label_4->setText("Displacement Y");
  label_5->setText("Displacement Z");
  label_6->setText("Rotation X");
  label_7->setText("Rotation Y");
  label_8->setText("Rotation Z");
  label_9->setText("OP");
  label_10->setText("Amplitude ID");
  label_11->setText("Time Delay");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();
  comboBox_9 = new QComboBox();
  comboBox_9->addItem("");
  comboBox_9->addItem("mod");
  comboBox_9->addItem("new");
  lineEdit_10 = new QLineEdit();
  lineEdit_11 = new QLineEdit();

  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Unchanged / number / free / fixed");
  lineEdit_4->setPlaceholderText("Unchanged / number / free / fixed");
  lineEdit_5->setPlaceholderText("Unchanged / number / free / fixed");
  lineEdit_6->setPlaceholderText("Unchanged / number / free / fixed");
  lineEdit_7->setPlaceholderText("Unchanged / number / free / fixed");
  lineEdit_8->setPlaceholderText("Unchanged / number / free / fixed");

  lineEdit_10->setPlaceholderText("Optional");
  lineEdit_11->setPlaceholderText("Optional");

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addLayout(HBoxLayout_9);
  VBoxLayout->addLayout(HBoxLayout_10);
  VBoxLayout->addLayout(HBoxLayout_11);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(lineEdit_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(comboBox_9);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(lineEdit_10);
  HBoxLayout_11->addWidget(label_11);
  HBoxLayout_11->addWidget(lineEdit_11);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;

}

BCsDisplacementsModifyPanel::~BCsDisplacementsModifyPanel()
{}

void BCsDisplacementsModifyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if (!lineEdit_0->text().trimmed().isEmpty())
  {
    command.append("ccx modify displacement " + lineEdit_0->text());
    if (!lineEdit_2->text().trimmed().isEmpty())
    {
      command.append(" nodeset " + lineEdit_2->text());
    }
    if (!lineEdit_3->text().trimmed().isEmpty())
    {
      command.append(" dof_1 " + lineEdit_3->text());
    }
    if (!lineEdit_4->text().trimmed().isEmpty())
    {
      command.append(" dof_2 " + lineEdit_4->text());
    }
    if (!lineEdit_5->text().trimmed().isEmpty())
    {
      command.append(" dof_3 " + lineEdit_5->text());
    }
    if (!lineEdit_6->text().trimmed().isEmpty())
    {
      command.append(" dof_4 " + lineEdit_6->text());
    }
    if (!lineEdit_7->text().trimmed().isEmpty())
    {
      command.append(" dof_5 " + lineEdit_7->text());
    }
    if (!lineEdit_8->text().trimmed().isEmpty())
    {
      command.append(" dof_6 " + lineEdit_8->text());
    }
    if (comboBox_9->currentIndex() != 0)
    {
      command.append(" op " + comboBox_9->currentText());
    }
    if (!lineEdit_10->text().trimmed().isEmpty())
    {
      command.append(" amplitude " + lineEdit_10->text());
    }
    if (!lineEdit_11->text().trimmed().isEmpty())
    {
      command.append(" timedelay " + lineEdit_11->text());
    }
    if (!lineEdit_1->text().isEmpty())
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
  }

  if (command != "")
  {
    commands.push_back(command);
    lineEdit_0->setText("");
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
    lineEdit_4->setText("");
    lineEdit_5->setText("");
    lineEdit_6->setText("");
    lineEdit_7->setText("");
    lineEdit_8->setText("");
    comboBox_9->setCurrentIndex(0);
    lineEdit_10->setText("");
    lineEdit_11->setText("");
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}
