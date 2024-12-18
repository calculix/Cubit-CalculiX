#include "LoadsTrajectoryCreatePanel.hpp"
#include "CalculiXCoreInterface.hpp"
#include "PanelTable.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"


LoadsTrajectoryCreatePanel::LoadsTrajectoryCreatePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_10 = new QFrame();
  frame_10->setMinimumSize(1,300);
  frame_10->setLineWidth(1);
  frame_10->setMidLineWidth(0);
  frame_10->setFrameStyle(QFrame::Box | QFrame::Raised);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();
  HBoxLayout_7 = new QHBoxLayout();
  HBoxLayout_8 = new QHBoxLayout();
  HBoxLayout_9 = new QHBoxLayout();
  HBoxLayout_10 = new QHBoxLayout(frame_10);
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  label_1->setText("Name");
  label_2->setText("Load Type");
  label_3->setText("Curve");
  label_4->setText("Vertex ID");
  label_5->setText("Surface IDs");
  label_6->setText("Direction");
  label_7->setText("Time Begin");
  label_8->setText("Time End");
  label_9->setText("OP");
  lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  comboBox_2->addItem("Heatflux");
  comboBox_2->addItem("Pressure");
  PickWidget_3 = new PickWidget();
  PickWidget_3->setPickType(PickWidget::Curve);
  PickWidget_3->activate();
  PickWidget_4 = new PickWidget();
  PickWidget_4->setPickType(PickWidget::Vertex);
  PickWidget_4->activate();
  PickWidget_5 = new PickWidget();
  PickWidget_5->setPickType(PickWidget::Surface);
  PickWidget_5->activate();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();
  comboBox_9 = new QComboBox();
  comboBox_9->addItem("");
  comboBox_9->addItem("mod");
  comboBox_9->addItem("new");
  widget_10 =  new PanelTable(nullptr);
  matrix.clear();
  widget_10->update({"Ray Radius","Magnitude"},matrix);
  widget_10->panel_type = "trajectory";
  widget_10->setMinimumSize(200,160);
  
  lineEdit_1->setPlaceholderText("Optional");
  PickWidget_4->setPlaceholderText("Start Vertex");
  lineEdit_6->setPlaceholderText("<x> <y> <z>");
  
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
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addLayout(HBoxLayout_9);
  VBoxLayout->addWidget(frame_10);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(comboBox_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(PickWidget_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(PickWidget_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(PickWidget_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(lineEdit_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(comboBox_9);
  HBoxLayout_10->addWidget(widget_10);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  widget_10->show();

  isInitialized = true;
}

LoadsTrajectoryCreatePanel::~LoadsTrajectoryCreatePanel()
{}

void LoadsTrajectoryCreatePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";
  matrix = widget_10->getMatrix();

  if ((PickWidget_3->text()!="") && (PickWidget_4->text()!="") && (PickWidget_5->text()!="") && (lineEdit_6->text()!="") && (lineEdit_7->text()!="") && (lineEdit_8->text()!="") && (matrix.size()>0))
  {
    command.append("ccx create trajectory " + comboBox_2->currentText() + "  curve " + PickWidget_3->text() + " vertex " + PickWidget_4->text()); 

    std::string pickwidget_text = PickWidget_5->text().toUtf8().data();
    std::vector<int> surface_ids = CubitInterface::parse_cubit_list("surface", pickwidget_text);
    command.append(" surface");
    for (size_t i = 0; i < surface_ids.size(); i++)
    {
      command.append(" " + QString::number(surface_ids[i]));
    }
    
    command.append(" direction " + lineEdit_6->text());
    command.append(" magnitude ");
    
    for (size_t i = 0; i < matrix.size(); i++)
    {
      command.append(QString::number(matrix[i][1]) + " ");
    }

    command.append(" time_begin " +lineEdit_7->text());
    command.append(" time_end " +lineEdit_8->text());
    command.append(" radius ");
    
    for (size_t i = 0; i < matrix.size(); i++)
    {
      command.append(QString::number(matrix[i][0]) + " ");
    }

    if (comboBox_9->currentIndex()!=0)
    {
      command.append(" op " + comboBox_9->currentText());
    }

    if (lineEdit_1->text()!="")
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    comboBox_2->setCurrentIndex(0);
    PickWidget_3->setText("");
    PickWidget_4->setText("");
    PickWidget_5->setText("");
    lineEdit_6->setText("");
    lineEdit_7->setText("");
    lineEdit_8->setText("");    
    comboBox_9->setCurrentIndex(0);
    matrix.clear();
    widget_10->update({"Ray Radius","Magnitude"},matrix);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}
