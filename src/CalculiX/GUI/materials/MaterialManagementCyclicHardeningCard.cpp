#include "MaterialManagementCyclicHardeningCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementCyclicHardeningCard::MaterialManagementCyclicHardeningCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Cyclic Hardening Card");
  table_cyclic_hardening = new MaterialManagementTable(nullptr,"CCX_CYCLIC_HARDENING_CYCLIC_HARDENING");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addWidget(table_cyclic_hardening);
  VBoxLayout->addItem(vertical_spacer);

  isInitialized = true;
}

MaterialManagementCyclicHardeningCard::~MaterialManagementCyclicHardeningCard()
{
}

void MaterialManagementCyclicHardeningCard::update(MaterialManagementItem *material)
{
  this->material = material;

  table_cyclic_hardening->update(material);
}