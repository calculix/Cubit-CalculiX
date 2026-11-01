#include "MaterialManagementSpecificHeatCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementSpecificHeatCard::MaterialManagementSpecificHeatCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Specific Heat Card");
  table_specific_heat = new MaterialManagementTable(nullptr,"CCX_SPECIFIC_HEAT_SPECIFIC_HEAT");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addWidget(table_specific_heat);
  VBoxLayout->addItem(vertical_spacer);

  isInitialized = true;
}

MaterialManagementSpecificHeatCard::~MaterialManagementSpecificHeatCard()
{
}

void MaterialManagementSpecificHeatCard::update(MaterialManagementItem *material)
{
  this->material = material;

  table_specific_heat->update(material);
}