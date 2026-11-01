#include "LoadsPressuresCreatePanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

LoadsPressuresCreatePanel::LoadsPressuresCreatePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if (isInitialized)
  {
    return;
  }

  int labelWidth = 120;

  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);

  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();

  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();

  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);

  label_1->setText("Name");
  label_2->setText("Sideset ID");
  label_3->setText("Magnitude");
  label_4->setText("OP");
  label_5->setText("Amplitude ID");
  label_6->setText("Time Delay");

  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();

  comboBox_4 = new QComboBox();
  comboBox_4->addItem("");
  comboBox_4->addItem("mod");
  comboBox_4->addItem("new");

  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();

  lineEdit_3->setText("0");
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(comboBox_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this, SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

LoadsPressuresCreatePanel::~LoadsPressuresCreatePanel()
{}

void LoadsPressuresCreatePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if (!lineEdit_2->text().trimmed().isEmpty() &&
      !lineEdit_3->text().trimmed().isEmpty())
  {
    command.append("ccx create pressure sideset " + lineEdit_2->text());
    command.append(" magnitude " + lineEdit_3->text());

    if (comboBox_4->currentIndex() != 0)
    {
      command.append(" op " + comboBox_4->currentText());
    }
    if (!lineEdit_5->text().trimmed().isEmpty())
    {
      command.append(" amplitude " + lineEdit_5->text());
    }
    if (!lineEdit_6->text().trimmed().isEmpty())
    {
      command.append(" timedelay " + lineEdit_6->text());
    }
    if (!lineEdit_1->text().isEmpty())
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
  }

  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("0");
    comboBox_4->setCurrentIndex(0);
    lineEdit_5->setText("");
    lineEdit_6->setText("");
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}
