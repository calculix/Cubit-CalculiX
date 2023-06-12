#include "CCXBlocksElementTypePanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"
#include "PickWidget.hpp"


CCXBlocksElementType::CCXBlocksElementType(QWidget *parent) :
  QWidget(parent)
{
 if(isInitialized)
    return;
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_Tab = new QHBoxLayout();
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Block ID");
  PickWidget_1 = new PickWidget();
  PickWidget_1->setPickType(PickWidget::Block);
  PickWidget_1->activate();
  //lineEdit_1->setPlaceholderText("Optional");
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_Tab);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(PickWidget_1);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  //tabs
  
  widget_beamtruss = new QWidget();
  widget_shellmembrane = new QWidget();
  widget_plainstressstrain = new QWidget();
  widget_axi = new QWidget();
  widget_volume = new QWidget();
  GridLayout_beamtruss = new QGridLayout(widget_beamtruss);
  GridLayout_shellmembrane = new QGridLayout(widget_shellmembrane);
  GridLayout_plainstressstrain = new QGridLayout(widget_plainstressstrain);
  GridLayout_axi = new QGridLayout(widget_axi);
  GridLayout_volume = new QGridLayout(widget_volume);
  std::vector<std::string> beamtruss;
  std::vector<std::string> shellmembrane;
  std::vector<std::string> plainstressstrain;
  std::vector<std::string> axi;
  std::vector<std::string> volume;

  //volume
  volume.push_back("C3D4");
  volume.push_back("C3D6");
  volume.push_back("C3D8");
  volume.push_back("C3D8R");
  volume.push_back("C3D8I");
  volume.push_back("C3D10");
  volume.push_back("C3D10T");
  volume.push_back("C3D15");
  volume.push_back("C3D20");
  volume.push_back("C3D20R");
  //shell
  shellmembrane.push_back("S3");
  shellmembrane.push_back("S4");
  shellmembrane.push_back("S4R");
  shellmembrane.push_back("S6");
  shellmembrane.push_back("S8");
  shellmembrane.push_back("S8R");
  //membrane
  shellmembrane.push_back("M3D3");
  shellmembrane.push_back("M3D4");
  shellmembrane.push_back("M3D4R");
  shellmembrane.push_back("M3D6");
  shellmembrane.push_back("M3D8");
  shellmembrane.push_back("M3D8R");

  // plane stress
  plainstressstrain.push_back("CPS3");
  plainstressstrain.push_back("CPS4");
  plainstressstrain.push_back("CPS4R");
  plainstressstrain.push_back("CPS6");
  plainstressstrain.push_back("CPS8");
  plainstressstrain.push_back("CPS8R");
  // plane strain
  plainstressstrain.push_back("CPE3");
  plainstressstrain.push_back("CPE4");
  plainstressstrain.push_back("CPE4R");
  plainstressstrain.push_back("CPE6");
  plainstressstrain.push_back("CPE8");
  plainstressstrain.push_back("CPE8R");
  // axisymetric elements
  axi.push_back("CAX3");
  axi.push_back("CAX4");
  axi.push_back("CAX4R");
  axi.push_back("CAX6");
  axi.push_back("CAX8");
  axi.push_back("CAX8R");
  // beam elements
  beamtruss.push_back("B21");
  beamtruss.push_back("B31");
  beamtruss.push_back("B31R");
  beamtruss.push_back("B32");
  beamtruss.push_back("B32R");
  // truss elements
  beamtruss.push_back("T2D2");
  beamtruss.push_back("T3D2");
  beamtruss.push_back("T3D3");

  int ir=0;
  int ic=0;
  int ir_max=4;
  int ic_max=1;

  //std::vector<QRadioButton*> RadioButton_beamtruss;
  std::vector<QVBoxLayout*> VBoxLayout_beamtruss;
  for (size_t i = 0; i < ic_max+1; i++)
  {
    VBoxLayout_beamtruss.push_back(new QVBoxLayout());
  }
  GridLayout_beamtruss->addLayout(VBoxLayout_beamtruss[ic],0,ic,Qt::AlignTop);
  for (size_t i = 0; i < beamtruss.size(); i++)
  {
    RadioButton_beamtruss.push_back(new QRadioButton());
    RadioButton_beamtruss.back()->setText(QString::fromStdString(beamtruss[i]));
    group.addButton(RadioButton_beamtruss.back(),-1);
    
    QHBoxLayout* newHBoxLayout = new QHBoxLayout();
    newHBoxLayout->addWidget(RadioButton_beamtruss.back());
    VBoxLayout_beamtruss[ic]->addLayout(newHBoxLayout);
    if ((ir==ir_max))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_beamtruss[ic]->addItem(new_vertical_spacer);
      if ((i!=beamtruss.size()-1))
      {
        GridLayout_beamtruss->addLayout(VBoxLayout_beamtruss[ic+1],0,ic+1,Qt::AlignTop);
      }
      ic+=1;
      ir=-1;
    }else if ((i==beamtruss.size()-1))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_beamtruss[ic]->addItem(new_vertical_spacer);
    }
    
    ir+=1;
  }
  
  //std::vector<QRadioButton*> RadioButton_shellmembrane;
  ir=0;
  ic=0;
  ir_max=5;
  ic_max=5;
  std::vector<QVBoxLayout*> VBoxLayout_shellmembrane;
  for (size_t i = 0; i < ic_max+1; i++)
  {
    VBoxLayout_shellmembrane.push_back(new QVBoxLayout());
  }
  GridLayout_shellmembrane->addLayout(VBoxLayout_shellmembrane[ic],0,ic,Qt::AlignTop);
  for (size_t i = 0; i < shellmembrane.size(); i++)
  {
    RadioButton_shellmembrane.push_back(new QRadioButton());
    RadioButton_shellmembrane.back()->setText(QString::fromStdString(shellmembrane[i]));
    group.addButton(RadioButton_shellmembrane.back(),-1);
    
    QHBoxLayout* newHBoxLayout = new QHBoxLayout();
    newHBoxLayout->addWidget(RadioButton_shellmembrane.back());
    VBoxLayout_shellmembrane[ic]->addLayout(newHBoxLayout);
    if ((ir==ir_max))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_shellmembrane[ic]->addItem(new_vertical_spacer);
      if ((i!=shellmembrane.size()-1))
      {
        GridLayout_shellmembrane->addLayout(VBoxLayout_shellmembrane[ic+1],0,ic+1,Qt::AlignTop);
      }
      ic+=1;
      ir=-1;
    }else if ((i==shellmembrane.size()-1))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_shellmembrane[ic]->addItem(new_vertical_spacer);
    }
    ir+=1;
  }
  
  //std::vector<QRadioButton*> RadioButton_plainstressstrain;
  ir=0;
  ic=0;
  ir_max=5;
  ic_max=5;
  std::vector<QVBoxLayout*> VBoxLayout_plainstressstrain;
  for (size_t i = 0; i < ic_max+1; i++)
  {
    VBoxLayout_plainstressstrain.push_back(new QVBoxLayout());
  }
  GridLayout_plainstressstrain->addLayout(VBoxLayout_plainstressstrain[ic],0,ic,Qt::AlignTop);
  for (size_t i = 0; i < plainstressstrain.size(); i++)
  {
    RadioButton_plainstressstrain.push_back(new QRadioButton());
    RadioButton_plainstressstrain.back()->setText(QString::fromStdString(plainstressstrain[i]));
    group.addButton(RadioButton_plainstressstrain.back(),-1);
    
    QHBoxLayout* newHBoxLayout = new QHBoxLayout();
    newHBoxLayout->addWidget(RadioButton_plainstressstrain.back());
    VBoxLayout_plainstressstrain[ic]->addLayout(newHBoxLayout);
    if ((ir==ir_max))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_plainstressstrain[ic]->addItem(new_vertical_spacer);
      if ((i!=plainstressstrain.size()-1))
      {
        GridLayout_plainstressstrain->addLayout(VBoxLayout_plainstressstrain[ic+1],0,ic+1,Qt::AlignTop);
      }
      ic+=1;
      ir=-1;
    }else if ((i==plainstressstrain.size()-1))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_plainstressstrain[ic]->addItem(new_vertical_spacer);
    }
    ir+=1;
  }
  //std::vector<QRadioButton*> RadioButton_axi;
  ir=0;
  ic=0;
  ir_max=5;
  ic_max=5;
  std::vector<QVBoxLayout*> VBoxLayout_axi;
  for (size_t i = 0; i < ic_max+1; i++)
  {
    VBoxLayout_axi.push_back(new QVBoxLayout());
  }
  GridLayout_axi->addLayout(VBoxLayout_axi[ic],0,ic,Qt::AlignTop);
  for (size_t i = 0; i < axi.size(); i++)
  {
    RadioButton_axi.push_back(new QRadioButton());
    RadioButton_axi.back()->setText(QString::fromStdString(axi[i]));
    group.addButton(RadioButton_axi.back(),-1);
    
    QHBoxLayout* newHBoxLayout = new QHBoxLayout();
    newHBoxLayout->addWidget(RadioButton_axi.back());
    VBoxLayout_axi[ic]->addLayout(newHBoxLayout);
    if ((ir==ir_max))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_axi[ic]->addItem(new_vertical_spacer);
      if ((i!=axi.size()-1))
      {
        GridLayout_axi->addLayout(VBoxLayout_axi[ic+1],0,ic+1,Qt::AlignTop);
      }
      ic+=1;
      ir=-1;
    }else if ((i==axi.size()-1))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_axi[ic]->addItem(new_vertical_spacer);
    }
    ir+=1;
  }
  //std::vector<QRadioButton*> RadioButton_volume;
  ir=0;
  ic=0;
  ir_max=5;
  ic_max=5;
  std::vector<QVBoxLayout*> VBoxLayout_volume;
  for (size_t i = 0; i < ic_max+1; i++)
  {
    VBoxLayout_volume.push_back(new QVBoxLayout());
  }
  GridLayout_volume->addLayout(VBoxLayout_volume[ic],0,ic,Qt::AlignTop);
  for (size_t i = 0; i < volume.size(); i++)
  {
    RadioButton_volume.push_back(new QRadioButton());
    RadioButton_volume.back()->setText(QString::fromStdString(volume[i]));
    group.addButton(RadioButton_volume.back(),-1);
    
    QHBoxLayout* newHBoxLayout = new QHBoxLayout();
    newHBoxLayout->addWidget(RadioButton_volume.back());
    VBoxLayout_volume[ic]->addLayout(newHBoxLayout);
    if ((ir==ir_max))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_volume[ic]->addItem(new_vertical_spacer);
      if ((i!=volume.size()-1))
      {
        GridLayout_volume->addLayout(VBoxLayout_volume[ic+1],0,ic+1,Qt::AlignTop);
      }
      ic+=1;
      ir=-1;
    }else if ((i==volume.size()-1))
    {
      QSpacerItem* new_vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
      VBoxLayout_volume[ic]->addItem(new_vertical_spacer);
    }
    ir+=1;
  }

  //tab widget
  TabWidget = new QTabWidget();
  TabWidget->addTab(widget_volume,"Volume");
  TabWidget->addTab(widget_shellmembrane,"Shell/Membrane");
  TabWidget->addTab(widget_beamtruss,"Beam/Truss");
  TabWidget->addTab(widget_plainstressstrain,"Plain Stress/Strain");
  TabWidget->addTab(widget_axi,"Axisymmetric");
  HBoxLayout_Tab->addWidget(TabWidget);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

CCXBlocksElementType::~CCXBlocksElementType()
{}

void CCXBlocksElementType::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if (group.exclusive())
  {
    QRadioButton *rb = dynamic_cast<QRadioButton*>(group.checkedButton());
    if (rb)
    {
      if (PickWidget_1->text()!="")
      {
        command.append("ccx block " + PickWidget_1->text() + " element_type "  + rb->text());
      }
    }
  }

  if (command != "")
  {
    commands.push_back(command);
    PickWidget_1->setText("");
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
