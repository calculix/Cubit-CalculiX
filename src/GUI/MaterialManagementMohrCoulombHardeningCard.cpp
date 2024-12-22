#include "MaterialManagementMohrCoulombHardeningCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementMohrCoulombHardeningCard::MaterialManagementMohrCoulombHardeningCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Mohr Coulomb Hardening Card");
  table_mohr_coulomb_hardening = new MaterialManagementTable(nullptr,"CCX_MOHR_COULOMB_HARDENING_MOHR_COULOMB_HARDENING");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addWidget(table_mohr_coulomb_hardening);
  VBoxLayout->addItem(vertical_spacer);

  isInitialized = true;
}

MaterialManagementMohrCoulombHardeningCard::~MaterialManagementMohrCoulombHardeningCard()
{
}

void MaterialManagementMohrCoulombHardeningCard::update(MaterialManagementItem *material)
{
  this->material = material;

  table_mohr_coulomb_hardening->update(material);
}