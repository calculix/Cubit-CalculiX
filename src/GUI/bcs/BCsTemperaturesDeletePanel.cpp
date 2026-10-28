#include "BCsTemperaturesDeletePanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

BCsTemperaturesDeletePanel::BCsTemperaturesDeletePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if (isInitialized)
  {
    return;
  }

  int labelWidth = 150;

  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);

  HBoxLayout_1 = new QHBoxLayout();
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Temperature ID");
  lineEdit_1 = new QLineEdit();

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this, SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

BCsTemperaturesDeletePanel::~BCsTemperaturesDeletePanel()
{}

void BCsTemperaturesDeletePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if (!lineEdit_1->text().trimmed().isEmpty())
  {
    command.append("ccx delete temperature " + lineEdit_1->text());
  }

  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}
