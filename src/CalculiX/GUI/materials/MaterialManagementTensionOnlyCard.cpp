#include "MaterialManagementTensionOnlyCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementTensionOnlyCard::MaterialManagementTensionOnlyCard(QWidget* parent, MaterialManagementItem *material):
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
  label_title->setText("Tension Only Card");
  table_tension_only = new MaterialManagementTable(nullptr,"CCX_TENSION_ONLY_TENSION_ONLY");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addWidget(table_tension_only);
  VBoxLayout->addItem(vertical_spacer);

  isInitialized = true;
}

MaterialManagementTensionOnlyCard::~MaterialManagementTensionOnlyCard()
{
}

void MaterialManagementTensionOnlyCard::update(MaterialManagementItem *material)
{
  this->material = material;

  table_tension_only->update(material);
}