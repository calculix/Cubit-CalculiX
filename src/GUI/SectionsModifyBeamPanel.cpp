#include "SectionsModifyBeamPanel.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


SectionsModifyBeamPanel::SectionsModifyBeamPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_beam_type = new QHBoxLayout();
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
  label_0 = new QLabel();
  label_beam_type = new QLabel();
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
  label_0->setFixedWidth(labelWidth);
  label_beam_type->setFixedWidth(labelWidth);
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
  label_12->setFixedWidth(labelWidth);
  label_13->setFixedWidth(labelWidth);
  label_14->setFixedWidth(labelWidth);
  label_0->setText("Section ID");
  label_beam_type->setText("Beam Type");
  label_1->setText("Block ID");
  label_2->setText("Material ID");
  label_3->setText("Parameter 1");
  label_4->setText("Parameter 2");
  label_5->setText("Parameter 3");
  label_6->setText("Parameter 4");
  label_7->setText("Parameter 5");
  label_8->setText("Parameter 6");
  label_9->setText("x");
  label_10->setText("y");
  label_11->setText("z");
  label_12->setText("Orientation ID");
  label_13->setText("Offset 1");
  label_14->setText("Offset 2");
  lineEdit_0 = new QLineEdit();
  beam_type = new QComboBox();
  beam_type->addItem("");
  beam_type->addItem("RECT");
  beam_type->addItem("CIRC");
  beam_type->addItem("PIPE");
  beam_type->addItem("BOX");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
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
  //lineEdit_0->setPlaceholderText("Optional");
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("only for BOX");
  lineEdit_6->setPlaceholderText("only for BOX");
  lineEdit_7->setPlaceholderText("only for BOX");
  lineEdit_8->setPlaceholderText("only for BOX");
  lineEdit_9->setPlaceholderText("Optional");
  lineEdit_10->setPlaceholderText("Optional");
  lineEdit_11->setPlaceholderText("Optional");
  lineEdit_12->setPlaceholderText("Optional");
  lineEdit_13->setPlaceholderText("Optional");
  lineEdit_14->setPlaceholderText("Optional");
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_beam_type);
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
  VBoxLayout->addLayout(HBoxLayout_12);
  VBoxLayout->addLayout(HBoxLayout_13);
  VBoxLayout->addLayout(HBoxLayout_14);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_beam_type->addWidget(label_beam_type);
  HBoxLayout_beam_type->addWidget(beam_type);
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
  HBoxLayout_9->addWidget(lineEdit_9);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(lineEdit_10);
  HBoxLayout_11->addWidget(label_11);
  HBoxLayout_11->addWidget(lineEdit_11);
  HBoxLayout_12->addWidget(label_12);
  HBoxLayout_12->addWidget(lineEdit_12);
  HBoxLayout_13->addWidget(label_13);
  HBoxLayout_13->addWidget(lineEdit_13);
  HBoxLayout_14->addWidget(label_14);
  HBoxLayout_14->addWidget(lineEdit_14);
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

SectionsModifyBeamPanel::~SectionsModifyBeamPanel()
{}

void SectionsModifyBeamPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify section beam " + lineEdit_0->text());
    
    
    if (beam_type->currentIndex()==1)
    {
      command.append(" beam_type rect ");
    }else if (beam_type->currentIndex()==2)
    {
      command.append(" beam_type circ ");
    }else if (beam_type->currentIndex()==3)
    {
      command.append(" beam_type pipe ");
    }else if (beam_type->currentIndex()==4)
    {
      command.append(" beam_type box ");
    }
    
    if (lineEdit_1->text()!="")
    {
      command.append(" block " + lineEdit_1->text());
    }
    if (lineEdit_2->text()!="")
    {
      command.append(" material " + lineEdit_2->text());
    }
    if (lineEdit_3->text()!="")
    {
      command.append(" parameter1 " + lineEdit_3->text());
    }
    if (lineEdit_4->text()!="")
    {
      command.append(" parameter2 " + lineEdit_4->text());
    }
    if (lineEdit_5->text()!="")
    {
      command.append(" parameter3 " + lineEdit_5->text());
    }
    if (lineEdit_6->text()!="")
    {
      command.append(" parameter4 " + lineEdit_6->text());
    }
    if (lineEdit_7->text()!="")
    {
      command.append(" parameter5 " + lineEdit_7->text());
    }
    if (lineEdit_8->text()!="")
    {
      command.append(" parameter6 " + lineEdit_8->text());
    }
    if (lineEdit_9->text()!="")
    {
      command.append(" x " + lineEdit_9->text());
    }
    if (lineEdit_10->text()!="")
    {
      command.append(" y " + lineEdit_10->text());
    }
    if (lineEdit_11->text()!="")
    {
      command.append(" z " + lineEdit_11->text());
    }
    if (lineEdit_12->text()!="")
    {
      command.append(" orientation " + lineEdit_12->text());
    }
    if (lineEdit_13->text()!="")
    {
      command.append(" offset1 " + lineEdit_13->text());
    }
    if (lineEdit_14->text()!="")
    {
      command.append(" offset2 " + lineEdit_14->text());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    beam_type->setCurrentIndex(0);
    lineEdit_0->setText("");
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
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
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}
