#include "MaterialManagementExpansionCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementExpansionCard::MaterialManagementExpansionCard(QWidget* parent, MaterialManagementItem *material):
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
  horizontal_spacer_zero = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  horizontal_spacer_type = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  label_title = new QLabel();
  label_title->setText("Expansion Card");
  label_zero = new QLabel();
  label_zero->setText("Reference Temperature");
  lineEdit_zero = new QLineEdit();
  lineEdit_zero->setFixedWidth(65);
  label_type = new QLabel();
  label_type->setText("Material Behavior");
  expansion_type = new QComboBox();
  expansion_type->addItem("Isotropic");
  expansion_type->addItem("Orthotropic");
  expansion_type->addItem("Anisotropic");
  
  table_isotropic = new MaterialManagementTable(nullptr,"CCX_EXPANSION_ISO_A_TEMPERATURE");
  table_orthotropic = new MaterialManagementTable(nullptr,"CCX_EXPANSION_ORTHO_CONSTANTS_VS_TEMPERATURE");
  table_anisotropic = new MaterialManagementTable(nullptr,"CCX_EXPANSION_ANISO_CONSTANTS_VS_TEMPERATURE");
  
  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_zero = new QHBoxLayout();
  HBoxLayout_zero->addWidget(label_zero);
  HBoxLayout_zero->addWidget(lineEdit_zero);
  HBoxLayout_zero->addItem(horizontal_spacer_zero);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(expansion_type);
  HBoxLayout_type->addItem(horizontal_spacer_type);
  VBoxLayout->addLayout(HBoxLayout_zero);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_isotropic);
  VBoxLayout->addWidget(table_orthotropic);
  VBoxLayout->addWidget(table_anisotropic);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(expansion_type, SIGNAL(currentIndexChanged(int)),this,SLOT(expansion_type_currentIndexChanged(int)));
  QObject::connect(lineEdit_zero, SIGNAL(textChanged(QString)),this,SLOT(lineEdit_zero_textChanged(QString)));

  isInitialized = true;
}

MaterialManagementExpansionCard::~MaterialManagementExpansionCard()
{
}

void MaterialManagementExpansionCard::update(MaterialManagementItem *material)
{
  this->material = material;

  if (this->material->getScalarPropertyGUI("CCX_EXPANSION_ISO_USE_CARD")==1)
  {
    expansion_type->setCurrentIndex(0);
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_EXPANSION_ORTHO_USE_CARD")==1)
  {
    expansion_type->setCurrentIndex(1);
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_EXPANSION_ANISO_USE_CARD")==1)
  {
    expansion_type->setCurrentIndex(2);
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();
  }else{
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ISO_USE_CARD", 1);
    expansion_type->setCurrentIndex(0);
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
  }
  if (this->material->getScalarPropertyGUI("CCX_EXPANSION_ZERO_USE_CARD")==1)
  {
    lineEdit_zero->setText(QString::number(this->material->getScalarPropertyGUI("CCX_EXPANSION_ZERO")));
  }else{
    lineEdit_zero->setText("");
  }
  
  table_isotropic->update(material);
  table_orthotropic->update(material);
  table_anisotropic->update(material);
}

void MaterialManagementExpansionCard::expansion_type_currentIndexChanged(int index)
{
  if (index == 0)
  {
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ISO_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ORTHO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ANISO_USE_CARD", 0);
  }else if (index == 1)
  {
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ORTHO_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ANISO_USE_CARD", 0);
  }else if (index == 2)
  {
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ISO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ORTHO_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ANISO_USE_CARD", 1);
  }
}

void MaterialManagementExpansionCard::lineEdit_zero_textChanged(QString text)
{
  if (text.toDouble()!=0.)
  {
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ZERO_USE_CARD",1);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ZERO",text.toDouble());
  } else {
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ZERO_USE_CARD",0);
    this->material->setScalarPropertyGUI("CCX_EXPANSION_ZERO",0);
  }
}