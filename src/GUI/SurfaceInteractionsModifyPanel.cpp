#include "SurfaceInteractionsModifyPanel.hpp"
#include "PanelTable.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


SurfaceInteractionsModifyPanel::SurfaceInteractionsModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  matrix.clear();
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_1 = new QFrame();
  frame_1->setMinimumSize(1,300);
  frame_1->setLineWidth(1);
  frame_1->setMidLineWidth(0);
  frame_1->setFrameStyle(QFrame::Box | QFrame::Raised);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  horizontal_spacer_1 = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  HBoxLayout_frame = new QHBoxLayout(frame_1);
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
  HBoxLayout_12 = new QHBoxLayout();
  HBoxLayout_13 = new QHBoxLayout();
  HBoxLayout_14 = new QHBoxLayout();
  comboBox_1 = new QComboBox();
  comboBox_1->addItem("Exponential");
  comboBox_1->addItem("Linear");
  comboBox_1->addItem("Tabular");
  comboBox_1->addItem("Tied");
  comboBox_1->addItem("Gap Conductance");
  comboBox_1->addItem("Gap Heat Generation");
  comboBox_1->addItem("Friction");
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
  label_12 = new QLabel();
  label_13 = new QLabel();
  label_14 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth-50);
  label_5->setFixedWidth(labelWidth-50);
  label_6->setFixedWidth(labelWidth-50);
  label_7->setFixedWidth(labelWidth-50);
  label_8->setFixedWidth(labelWidth-50);
  label_9->setFixedWidth(labelWidth-50);
  label_10->setFixedWidth(labelWidth+10);
  label_11->setFixedWidth(labelWidth+10);
  label_12->setFixedWidth(labelWidth+10);
  label_13->setFixedWidth(labelWidth-50);
  label_14->setFixedWidth(labelWidth-50);
  label_1->setText("Surface \nInteraction ID");
  label_2->setText("Name");
  label_3->setText("Surface Behavior:");
  label_4->setText("c0");
  label_5->setText("p0");
  label_6->setText("slope K");
  label_7->setText("sigmaINF");
  label_8->setText("c0");
  label_9->setText("slope K");
  label_10->setText("heat conversion \nfactor");
  label_11->setText("surface weighting \nfactor");
  label_12->setText("differential \ntangential velocity");
  label_13->setText("mu");
  label_14->setText("lambda");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();
  lineEdit_9 = new QLineEdit();
  lineEdit_10 = new QLineEdit();
  lineEdit_11 = new QLineEdit();
  lineEdit_12 = new QLineEdit();
  lineEdit_13 = new QLineEdit();
  lineEdit_14 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");
  lineEdit_7->setPlaceholderText("Optional");
  lineEdit_8->setPlaceholderText("Optional");
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addWidget(frame_1);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(comboBox_1);

  widget_1 =  new QWidget();
  VBoxLayout_1 = new QVBoxLayout(widget_1);
  vertical_spacer_1 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_1->addLayout(HBoxLayout_4);
  VBoxLayout_1->addLayout(HBoxLayout_5);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  VBoxLayout_1->addItem(vertical_spacer_1);

  widget_2 =  new QWidget();
  VBoxLayout_2 = new QVBoxLayout(widget_2);
  vertical_spacer_2 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_2->addLayout(HBoxLayout_6);
  VBoxLayout_2->addLayout(HBoxLayout_7);
  VBoxLayout_2->addLayout(HBoxLayout_8);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(lineEdit_8);
  VBoxLayout_2->addItem(vertical_spacer_2);

  widget_3 =  new PanelTable(nullptr);
  widget_3->update({"pressure","overclosure"},matrix);
  //widget_3->setColumnWidth(0,150);
  //widget_3->setMinimumSize(200,280);

  widget_4 =  new QWidget();
  VBoxLayout_3 = new QVBoxLayout(widget_4);
  vertical_spacer_3 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_3->addLayout(HBoxLayout_9);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);
  VBoxLayout_3->addItem(vertical_spacer_3);

  widget_5 =  new PanelTable(nullptr);
  widget_5->update({"Conductance","Contact pressure","Temperature"},matrix);
  //widget_5->setColumnWidth(1,200);
  //widget_5->setMinimumSize(300,280);

  widget_6 =  new QWidget();
  VBoxLayout_4 = new QVBoxLayout(widget_6);
  vertical_spacer_4 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_4->addLayout(HBoxLayout_10);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(lineEdit_10);
  VBoxLayout_4->addLayout(HBoxLayout_11);
  HBoxLayout_11->addWidget(label_11);
  HBoxLayout_11->addWidget(lineEdit_11);
  VBoxLayout_4->addLayout(HBoxLayout_12);
  HBoxLayout_12->addWidget(label_12);
  HBoxLayout_12->addWidget(lineEdit_12);
  VBoxLayout_4->addItem(vertical_spacer_4);

  widget_7 =  new QWidget();
  VBoxLayout_5 = new QVBoxLayout(widget_7);
  vertical_spacer_5 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_5->addLayout(HBoxLayout_13);
  HBoxLayout_13->addWidget(label_13);
  HBoxLayout_13->addWidget(lineEdit_13);
  VBoxLayout_5->addLayout(HBoxLayout_14);
  HBoxLayout_14->addWidget(label_14);
  HBoxLayout_14->addWidget(lineEdit_14);
  VBoxLayout_5->addItem(vertical_spacer_5);

  HBoxLayout_frame->addWidget(widget_1);
  HBoxLayout_frame->addWidget(widget_2);
  HBoxLayout_frame->addWidget(widget_3);
  HBoxLayout_frame->addWidget(widget_4);
  HBoxLayout_frame->addWidget(widget_5);
  HBoxLayout_frame->addWidget(widget_6);
  HBoxLayout_frame->addWidget(widget_7);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  comboBox_1->setCurrentIndex(0);
  widget_1->show();
  widget_2->hide();
  widget_3->hide();
  widget_4->hide();
  widget_5->hide();
  widget_6->hide();
  widget_7->hide();

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(comboBox_1, SIGNAL(currentIndexChanged(int)),this,  SLOT(comboBox_currentIndexChanged(int)));

  isInitialized = true;
}

SurfaceInteractionsModifyPanel::~SurfaceInteractionsModifyPanel()
{}

void SurfaceInteractionsModifyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify surfaceinteraction " + lineEdit_1->text());

    if (lineEdit_2->text()!="")
    {
      command.append(" name \"" + lineEdit_2->text() + "\"");
      commands.push_back(command);
      command = "ccx modify surfaceinteraction " + lineEdit_1->text();
    }
    
    if (comboBox_1->currentIndex()==0)
    {
      command.append(" exponential");
      if (lineEdit_4->text()!="")
      {
        command.append(" c0 " + lineEdit_4->text());
      }
      if (lineEdit_5->text()!="")
      {
        command.append(" p0 " + lineEdit_5->text());
      }
    }else if (comboBox_1->currentIndex()==1)
    {
      command.append(" linear");
      if (lineEdit_6->text()!="")
      {
        command.append(" slopeK " + lineEdit_6->text());
      }
      if (lineEdit_7->text()!="")
      {
        command.append(" sigmaINF " + lineEdit_7->text());
      }
      if (lineEdit_8->text()!="")
      {
        command.append(" c0 " + lineEdit_8->text());
      }
    }else if (comboBox_1->currentIndex()==2)
    {
      command.append(" tabular pressure_overclosure ");
      matrix = widget_3->getMatrix();
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii]) + " ");
        }
      }
    }else if (comboBox_1->currentIndex()==3)
    {
      command.append(" tied slopeK " + lineEdit_9->text());
    }else if (comboBox_1->currentIndex()==4)
    {
      command.append(" gapconductance conductance_contactpressure_temperature ");
      matrix = widget_5->getMatrix();
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii]) + " ");
        }
      }
    }else if (comboBox_1->currentIndex()==5)
    { 
      command.append(" gapheatgeneration");
      if (lineEdit_10->text()!="")
      {
        command.append(" heat_conversion_factor " + lineEdit_10->text());
      }
      if (lineEdit_11->text()!="")
      {
        command.append(" surface_weighing_factor " + lineEdit_11->text());
      }
      if (lineEdit_12->text()!="")
      {
        command.append(" differential_tangential_velocity " + lineEdit_12->text());
      }
    }else if (comboBox_1->currentIndex()==6)
    { 
      command.append(" friction");
      if (lineEdit_13->text()!="")
      {
        command.append(" mu " + lineEdit_13->text());
      }
      if (lineEdit_14->text()!="")
      {
        command.append(" lambda " + lineEdit_14->text());
      }
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    comboBox_1->setCurrentIndex(0);
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_4->setText("");
    lineEdit_5->setText("");
    lineEdit_6->setText("");
    lineEdit_7->setText("");
    lineEdit_8->setText("");
    lineEdit_9->setText("");
    lineEdit_10->setText("");
    lineEdit_11->setText("");
    lineEdit_12->setText("");
    lineEdit_13->setText("");
    lineEdit_14->setText("");
    matrix.clear();
    widget_3->update({"pressure","overclosure"},matrix);
    widget_5->update({"Conductance","Contact pressure","Temperature"},matrix);
  
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}


void SurfaceInteractionsModifyPanel::comboBox_currentIndexChanged(int index)
{
  widget_1->hide();
  widget_2->hide();
  widget_3->hide();
  widget_4->hide();
  widget_5->hide();
  widget_6->hide();
  widget_7->hide();
  if (index==0)
  {
    widget_1->show();
  }else if (index==1)
  {
    widget_2->show();
  }else if (index==2)
  {
    widget_3->show();
  }else if (index==3)
  {
    widget_4->show();
  }else if (index==4){
    widget_5->show();
  }else if (index==5){
    widget_6->show();
  }else if (index==6){
    widget_7->show();
  }  
}
