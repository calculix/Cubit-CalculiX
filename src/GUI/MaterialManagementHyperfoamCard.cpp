#include "MaterialManagementHyperfoamCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementHyperfoamCard::MaterialManagementHyperfoamCard(QWidget* parent, MaterialManagementItem *material):
  QWidget (parent),
  isInitialized(false)
{
  if(isInitialized)
    return;

  this->material = material;

  this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  HBoxLayout = new QHBoxLayout();
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  horizontal_spacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  horizontal_spacer_type = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  label_title = new QLabel();
  label_title->setText("Hyperfoam Card");
  label_type = new QLabel();
  label_type->setText("Material Behavior");
  hyperfoam_type = new QComboBox();
  hyperfoam_type->addItem("Hyperfoam N=1");
  hyperfoam_type->addItem("Hyperfoam N=2");
  hyperfoam_type->addItem("Hyperfoam N=3");
  
  table_n1 = new MaterialManagementTable(nullptr,"CCX_HYPERFOAM_HYPERFOAM_N1");
  table_n2 = new MaterialManagementTable(nullptr,"CCX_HYPERFOAM_HYPERFOAM_N2");
  table_n3 = new MaterialManagementTable(nullptr,"CCX_HYPERFOAM_HYPERFOAM_N3");
  
  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(hyperfoam_type);
  HBoxLayout_type->addItem(horizontal_spacer_type);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_n1);
  VBoxLayout->addWidget(table_n2);
  VBoxLayout->addWidget(table_n3);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(hyperfoam_type, SIGNAL(currentIndexChanged(int)),this,SLOT(hyperfoam_type_currentIndexChanged(int)));

  isInitialized = true;
}

MaterialManagementHyperfoamCard::~MaterialManagementHyperfoamCard()
{
}

void MaterialManagementHyperfoamCard::update(MaterialManagementItem *material)
{
  this->material = material;

  if (this->material->getScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD")==1)
  {
    hyperfoam_type->setCurrentIndex(0);
    table_n1->show();
    table_n2->hide();
    table_n3->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N2_USE_CARD")==1)
  {
    hyperfoam_type->setCurrentIndex(1);
    table_n1->hide();
    table_n2->show();
    table_n3->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N3_USE_CARD")==1)
  {
    hyperfoam_type->setCurrentIndex(2);
    table_n1->hide();
    table_n2->hide();
    table_n3->show();
  }else{
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD", 1);
    hyperfoam_type->setCurrentIndex(0);
    table_n1->show();
    table_n2->hide();
    table_n3->hide();
  }
  
  table_n1->update(material);
  table_n2->update(material);
  table_n3->update(material);
}

void MaterialManagementHyperfoamCard::hyperfoam_type_currentIndexChanged(int index)
{
  if (index == 0)
  {
    table_n1->show();
    table_n2->hide();
    table_n3->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N3_USE_CARD", 0);
  }else if (index == 1)
  {
    table_n1->hide();
    table_n2->show();
    table_n3->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N2_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N3_USE_CARD", 0);
  }else if (index == 2)
  {
    table_n1->hide();
    table_n2->hide();
    table_n3->show();
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERFOAM_HYPERFOAM_N3_USE_CARD", 1);
  }
}