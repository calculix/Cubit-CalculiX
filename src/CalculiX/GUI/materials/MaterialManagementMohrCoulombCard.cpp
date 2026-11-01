#include "MaterialManagementMohrCoulombCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementMohrCoulombCard::MaterialManagementMohrCoulombCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Mohr Coulomb Card");
  table_mohr_coulomb = new MaterialManagementTable(nullptr,"CCX_MOHR_COULOMB_MOHR_COULOMB");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addWidget(table_mohr_coulomb);
  VBoxLayout->addItem(vertical_spacer);

  isInitialized = true;
}

MaterialManagementMohrCoulombCard::~MaterialManagementMohrCoulombCard()
{
}

void MaterialManagementMohrCoulombCard::update(MaterialManagementItem *material)
{
  this->material = material;

  table_mohr_coulomb->update(material);
}