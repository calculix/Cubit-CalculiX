#include "MaterialManagementDampingCard.hpp"
#include "MaterialManagementItem.hpp"
#include "CubitMessage.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QSignalBlocker>
#include <cmath>

MaterialManagementDampingCard::MaterialManagementDampingCard(QWidget* parent, MaterialManagementItem *material):
  QWidget (parent),
  isInitialized(false) {
  this->material = material;

  this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  HBoxLayout = new QHBoxLayout();
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  horizontal_spacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  horizontal_spacer_structural = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  label_title = new QLabel();
  label_title->setText("Damping Card");
  label_structural = new QLabel();
  label_structural->setText("Structural");
  lineEdit_structural = new QLineEdit();
  lineEdit_structural->setFixedWidth(65);

  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  HBoxLayout_structural = new QHBoxLayout();
  HBoxLayout_structural->addWidget(label_structural);
  HBoxLayout_structural->addWidget(lineEdit_structural);
  HBoxLayout_structural->addItem(horizontal_spacer_structural);
  VBoxLayout->addWidget(label_title);
  VBoxLayout->addLayout(HBoxLayout_structural);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(lineEdit_structural, SIGNAL(textChanged(QString)),this,SLOT(lineEdit_structural_textChanged(QString)));

  isInitialized = true;
}

MaterialManagementDampingCard::~MaterialManagementDampingCard() {
}

void MaterialManagementDampingCard::update(MaterialManagementItem *material) {
  if (material == nullptr) {
    return;
  }
  if (this->material == material && material->hasPropertyError("CCX_DAMPING_DAMPING") && displayedGeneration == framework_units().generation()) {
    return;
  }
  if (material->hasPropertyError("CCX_DAMPING_DAMPING")) {
    PRINT_ERROR("Material display units changed. Invalid damping input was discarded; enter the value again.\n");
    material->clearPropertyError("CCX_DAMPING_DAMPING");
  }
  this->material = material;
  displayedGeneration = framework_units().generation();
  const QSignalBlocker blocker(lineEdit_structural);
  auto found = material->draft.cards.find("damping");
  if (found != material->draft.cards.end() && found->second.values.scalar.has_value()) {
    lineEdit_structural->setText(QString::number(material->getScalarPropertyGUI("CCX_DAMPING_DAMPING"), 'g', 17));
  } else {
    lineEdit_structural->clear();
  }
}

void MaterialManagementDampingCard::lineEdit_structural_textChanged(QString text) {
  if (material != nullptr && displayedGeneration != framework_units().generation()) {
    PRINT_ERROR("Material display units changed. The field has been refreshed; enter the value again.\n");
    update(material);
    return;
  }

  if (material == nullptr) {
    return;
  }
  if (text.trimmed().isEmpty()) {
    auto found = material->draft.cards.find("damping");
    if (found != material->draft.cards.end()) {
      found->second.values.scalar.reset();
    }
    material->clearPropertyError("CCX_DAMPING_DAMPING");
    return;
  }
  bool valid = false;
  const double value = text.toDouble(&valid);
  if (!valid || !std::isfinite(value)) {
    material->setPropertyError("CCX_DAMPING_DAMPING", "Structural damping must be a finite number.");
    PRINT_ERROR("Structural damping must be a finite number.\n");
    return;
  }
  material->setScalarPropertyGUI("CCX_DAMPING_DAMPING", value);
}
