#include "AmplitudesModifyPanel.hpp"
#include "PanelTable.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


AmplitudesModifyPanel::AmplitudesModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_5 = new QFrame();
  frame_5->setMinimumSize(1,300);
  frame_5->setLineWidth(1);
  frame_5->setMidLineWidth(0);
  frame_5->setFrameStyle(QFrame::Box | QFrame::Raised);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout(frame_5);
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_0->setText("Amplitude ID");
  label_1->setText("Name");
  label_2->setText("Shift x");
  label_3->setText("Shift y");
  label_4->setText("Time=TotalTime");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  checkBox_4 = new QCheckBox();
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  widget_5 =  new PanelTable(nullptr);
  matrix.clear();
  widget_5->update({"Time","Amplitude"},matrix);
  //widget_3->setColumnWidth(0,150);
  widget_5->panel_type = "amplitude";
  widget_5->setMinimumSize(200,160);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addWidget(frame_5);
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
  HBoxLayout_4->addWidget(checkBox_4);
  HBoxLayout_5->addWidget(widget_5);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  widget_5->show();
  checkBox_4->setChecked(false);

  isInitialized = true;
}

AmplitudesModifyPanel::~AmplitudesModifyPanel()
{}

void AmplitudesModifyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";
  matrix = widget_5->getMatrix();

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify amplitude "+ lineEdit_0->text());
    
    if (lineEdit_1->text()!="")
    {
      command.append(" name \"" +lineEdit_1->text() + "\"");
    }
    if (matrix.size()!=0)
    {
      command.append(" time_amplitude ");
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii]) + " ");
        }
      }
    }
    if (lineEdit_2->text()!="")
    {
      command.append(" shiftx " +lineEdit_2->text());
    }
    if (lineEdit_3->text()!="")
    {
      command.append(" shifty " +lineEdit_3->text());
    }
    if((checkBox_4->isChecked()))
    {
      command.append(" totaltime_yes");
    }else{
      command.append(" totaltime_no");
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_0->setText("");
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
    checkBox_4->setChecked(false);
    matrix.clear();
    widget_5->update({"Time","Amplitude"},matrix);
  }
  

  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[int(i)].toStdString().c_str());
  }
}
