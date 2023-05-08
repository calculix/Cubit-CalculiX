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

  this->setMinimumSize(700,300);
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
  table_isotropic = new MaterialManagementTable(this,"CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE");
  //table_orthotropic = new MaterialManagementTable(this,"ORTHO_CONSTANTS_VS_TEMPERATURE");
  //table_anisotropic = new MaterialManagementTable(this,"EC_CONSTANTS_VS_TEMPERATURE");
  //table_ec = new MaterialManagementTable(this,"ANISO_CONSTANTS_VS_TEMPERATURE");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(elastic_type);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_isotropic);
  //VBoxLayout->addWidget(table_orthotropic);
  //VBoxLayout->addWidget(table_anisotropic);
  //VBoxLayout->addWidget(table_ec);
  VBoxLayout->addItem(vertical_spacer);

  //table_isotropic->hide();
  //table_orthotropic->hide();
  //table_anisotropic->hide();
  //table_ec->hide();

  /*
    group_properties.push_back({material_card[1] + "ISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ISOTROPIC CARD");
    group_properties.push_back({material_card[1] + "ISO_MODULUS_VS_POISSON_VS_TEMPERATURE", "4", "3"});
    group_properties_description.push_back("ISOTROPIC:\nYoung's Modulus vs Poisson's ratio vs Temperature");
    group_properties.push_back({material_card[1] + "ORTHO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ORTHOTROPIC CARD");
    group_properties.push_back({material_card[1] + "ORTHO_CONSTANTS_VS_TEMPERATURE", "4", "10"});
    group_properties_description.push_back("ORTHOTROPIC:\nD1111,D1122,D2222,D1133,D2233,D3333,D1212,D1313,\nD2323,TEMPERATURE");
    group_properties.push_back({material_card[1] + "EC_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ENGINEERING CONSTANTS CARD");
    group_properties.push_back({material_card[1] + "EC_CONSTANTS_VS_TEMPERATURE", "4", "10"});
    group_properties_description.push_back("ENGINEERING CONSTANTS:\nE1,E2,E3,v12,v13,v23,G12,G13,\nG23,TEMPERATURE");
    group_properties.push_back({material_card[1] + "ANISO_USE_CARD", "1"});
    group_properties_description.push_back("SET 1 TO USE ELASTIC TYPE=ANISO CARD");
    group_properties.push_back({material_card[1] + "ANISO_CONSTANTS_VS_TEMPERATURE", "4", "22"});
    group_properties_description.push_back("ANISOTROPIC:\nD1111,D1122,D2222,D1133,D2233,D3333,D1112,D2212,\nD3312,D1212,D1113,D2213,D3313,D1213,D1313,D1123,\nD2223,D3323,D1223,D1323,D2323,TEMPERATURE");
  */

  //QObject::connect(elastic_type, SIGNAL(currentIndexChanged(int)),this,SLOT(elastic_type_currentIndexChanged(int)));

  isInitialized = true;
}

MaterialManagementElasticCard::~MaterialManagementElasticCard()
{
}

void MaterialManagementElasticCard::update(MaterialManagementItem *material)
{
  this->material = material;
  table_isotropic->update(material);
}

void MaterialManagementElasticCard::elastic_type_currentIndexChanged(int index)
{
  if (index == 0)
  {
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->hide();
  }else if (index == 1)
  {
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();
    table_ec->hide();
  }else if (index == 2)
  {
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();
    table_ec->hide();
  }else if (index == 3)
  {
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->hide();
    table_ec->show();
  }
}