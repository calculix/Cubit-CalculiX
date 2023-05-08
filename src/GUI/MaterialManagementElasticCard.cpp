#include "MaterialManagementElasticCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementElasticCard::MaterialManagementElasticCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Elastic Card");
  label_type = new QLabel();
  label_type->setText("Material Behavior");
  elastic_type = new QComboBox();
  elastic_type->addItem("Isotropic");
  elastic_type->addItem("Orthotropic");
  elastic_type->addItem("Anisotropic");
  elastic_type->addItem("Engineering Constants");
  table_isotropic = new MaterialManagementTable(nullptr,"CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE");
  table_orthotropic = new MaterialManagementTable(nullptr,"CCX_ELASTIC_ORTHO_CONSTANTS_VS_TEMPERATURE");
  table_anisotropic = new MaterialManagementTable(nullptr,"CCX_ELASTIC_EC_CONSTANTS_VS_TEMPERATURE");
  table_ec = new MaterialManagementTable(nullptr,"CCX_ELASTIC_ANISO_CONSTANTS_VS_TEMPERATURE");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(elastic_type);
  HBoxLayout_type->addItem(horizontal_spacer_type);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_isotropic);
  VBoxLayout->addWidget(table_orthotropic);
  VBoxLayout->addWidget(table_anisotropic);
  VBoxLayout->addWidget(table_ec);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(elastic_type, SIGNAL(currentIndexChanged(int)),this,SLOT(elastic_type_currentIndexChanged(int)));

  isInitialized = true;
}

MaterialManagementElasticCard::~MaterialManagementElasticCard()
{
}

void MaterialManagementElasticCard::update(MaterialManagementItem *material)
{
  this->material = material;

  if (this->material->getScalarPropertyGUI("CCX_ELASTIC_ISO_USE_CARD")==1)
  {
    elastic_type->setCurrentIndex(0);
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_ELASTIC_ORTHO_USE_CARD")==1)
  {
    elastic_type->setCurrentIndex(1);
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();
    table_ec->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_ELASTIC_ANISO_USE_CARD")==1)
  {
    elastic_type->setCurrentIndex(2);
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();
    table_ec->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_ELASTIC_EC_USE_CARD")==1)
  {
    elastic_type->setCurrentIndex(3);
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->show();
  }else{
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ISO_USE_CARD", 1);
    elastic_type->setCurrentIndex(0);
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->hide();
  }
  
  table_isotropic->update(material);
  table_orthotropic->update(material);
  table_anisotropic->update(material);
  table_ec->update(material);
}

void MaterialManagementElasticCard::elastic_type_currentIndexChanged(int index)
{
  if (index == 0)
  {
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->hide();
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ISO_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ORTHO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ANISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_EC_USE_CARD", 0);
  }else if (index == 1)
  {
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();
    table_ec->hide();
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ORTHO_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ANISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_EC_USE_CARD", 0);
  }else if (index == 2)
  {
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();
    table_ec->hide();
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ORTHO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ANISO_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_EC_USE_CARD", 0);
  }else if (index == 3)
  {
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->show();
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ORTHO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_ANISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_ELASTIC_EC_USE_CARD", 1);
  }
}