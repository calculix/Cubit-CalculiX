#include "MaterialManagementConductivityCard.hpp"
#include "MaterialManagementItem.hpp"
#include "CubitMessage.hpp"
#include <QSignalBlocker>
#include <cmath>
#include "MaterialManagementTable.hpp"

MaterialManagementConductivityCard::MaterialManagementConductivityCard(QWidget* parent, MaterialManagementItem *material):
  QWidget (parent),
  isInitialized(false) {
  this->material = material;

  this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  HBoxLayout = new QHBoxLayout();
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  horizontal_spacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  horizontal_spacer_type = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  label_title = new QLabel();
  label_title->setText("Conductivity Card");
  label_type = new QLabel();
  label_type->setText("Material Behavior");
  conductivity_type = new QComboBox();
  conductivity_type->addItem("Isotropic");
  conductivity_type->addItem("Orthotropic");
  conductivity_type->addItem("Anisotropic");

  table_isotropic = new MaterialManagementTable(nullptr,"CCX_CONDUCTIVITY_ISO_K_TEMPERATURE");
  table_orthotropic = new MaterialManagementTable(nullptr,"CCX_CONDUCTIVITY_ORTHO_CONSTANTS_VS_TEMPERATURE");
  table_anisotropic = new MaterialManagementTable(nullptr,"CCX_CONDUCTIVITY_ANISO_CONSTANTS_VS_TEMPERATURE");

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(conductivity_type);
  HBoxLayout_type->addItem(horizontal_spacer_type);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_isotropic);
  VBoxLayout->addWidget(table_orthotropic);
  VBoxLayout->addWidget(table_anisotropic);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(conductivity_type, SIGNAL(currentIndexChanged(int)),this,SLOT(conductivity_type_currentIndexChanged(int)));

  isInitialized = true;
}

MaterialManagementConductivityCard::~MaterialManagementConductivityCard() {
}

void MaterialManagementConductivityCard::update(MaterialManagementItem *material) {
  if (material == nullptr) {
    return;
  }
  const QSignalBlocker conductivity_typeBlocker(conductivity_type);

  this->material = material;

  if (this->material->modelSelectedGUI("conductivity", "iso")) {
    conductivity_type->setCurrentIndex(0);
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
  } else if (this->material->modelSelectedGUI("conductivity", "ortho")) {
    conductivity_type->setCurrentIndex(1);
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();
  } else if (this->material->modelSelectedGUI("conductivity", "aniso")) {
    conductivity_type->setCurrentIndex(2);
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();
  } else {

    conductivity_type->setCurrentIndex(0);
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
  }

  table_isotropic->update(material);
  table_orthotropic->update(material);
  table_anisotropic->update(material);
}

void MaterialManagementConductivityCard::conductivity_type_currentIndexChanged(int index) {
  if (this->material == nullptr) {
    return;
  }

  if (index == 0) {
    table_isotropic->show();
    table_orthotropic->hide();
    table_anisotropic->hide();
    this->material->selectModelGUI("conductivity", "iso");

  } else if (index == 1) {
    table_isotropic->hide();
    table_orthotropic->show();
    table_anisotropic->hide();

    this->material->selectModelGUI("conductivity", "ortho");

  } else if (index == 2) {
    table_isotropic->hide();
    table_orthotropic->hide();
    table_anisotropic->show();

    this->material->selectModelGUI("conductivity", "aniso");
  }

  table_anisotropic->update(this->material);
  table_isotropic->update(this->material);
  table_orthotropic->update(this->material);
}
