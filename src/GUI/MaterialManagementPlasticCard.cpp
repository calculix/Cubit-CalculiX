#include "MaterialManagementPlasticCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementPlasticCard::MaterialManagementPlasticCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Plastic Card");
  label_type = new QLabel();
  label_type->setText("Hardening Type");
  plastic_type = new QComboBox();
  plastic_type->addItem("Isotropic");
  plastic_type->addItem("Kinematic");
  table_isotropic = new MaterialManagementTable(nullptr,"CCX_PLASTIC_ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE");
  table_kinematic = new MaterialManagementTable(nullptr,"CCX_PLASTIC_KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(plastic_type);
  HBoxLayout_type->addItem(horizontal_spacer_type);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_isotropic);
  VBoxLayout->addWidget(table_kinematic);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(plastic_type, SIGNAL(currentIndexChanged(int)),this,SLOT(plastic_type_currentIndexChanged(int)));

  isInitialized = true;
}

MaterialManagementPlasticCard::~MaterialManagementPlasticCard()
{
}

void MaterialManagementPlasticCard::update(MaterialManagementItem *material)
{
  this->material = material;

  if (this->material->getScalarPropertyGUI("CCX_PLASTIC_ISO_USE_CARD")==1)
  {
    plastic_type->setCurrentIndex(0);
    table_isotropic->show();
    table_kinematic->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_PLASTIC_KIN_USE_CARD")==1)
  {
    plastic_type->setCurrentIndex(1);
    table_isotropic->hide();
    table_kinematic->show();
  }else{
    this->material->setScalarPropertyGUI("CCX_PLASTIC_ISO_USE_CARD", 1);
    plastic_type->setCurrentIndex(0);
    table_isotropic->show();
    table_kinematic->hide();
  }
  table_isotropic->update(material);
  table_kinematic->update(material);
}

void MaterialManagementPlasticCard::plastic_type_currentIndexChanged(int index)
{
  if (index == 0)
  {
    table_isotropic->show();
    table_kinematic->hide();
    this->material->setScalarPropertyGUI("CCX_PLASTIC_ISO_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_PLASTIC_KIN_USE_CARD", 0);
  }else if (index == 1)
  {
    table_isotropic->hide();
    table_kinematic->show();
    this->material->setScalarPropertyGUI("CCX_PLASTIC_ISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_PLASTIC_KIN_USE_CARD", 1);
  }
}