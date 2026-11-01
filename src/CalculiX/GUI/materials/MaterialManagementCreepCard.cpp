#include "MaterialManagementCreepCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementCreepCard::MaterialManagementCreepCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title = new QLabel();
  label_title->setText("Creep Card");
  table_creep = new MaterialManagementTable(nullptr,"CCX_CREEP_CREEP");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addWidget(table_creep);
  VBoxLayout->addItem(vertical_spacer);

  isInitialized = true;
}

MaterialManagementCreepCard::~MaterialManagementCreepCard()
{
}

void MaterialManagementCreepCard::update(MaterialManagementItem *material)
{
  this->material = material;

  table_creep->update(material);
}