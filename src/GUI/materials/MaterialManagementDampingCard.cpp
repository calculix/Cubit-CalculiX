#include "MaterialManagementDampingCard.hpp"
#include "MaterialManagementItem.hpp"

MaterialManagementDampingCard::MaterialManagementDampingCard(QWidget* parent, MaterialManagementItem *material):
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

MaterialManagementDampingCard::~MaterialManagementDampingCard()
{
}

void MaterialManagementDampingCard::update(MaterialManagementItem *material)
{
  this->material = material;
}

void MaterialManagementDampingCard::lineEdit_structural_textChanged(QString text)
{
  if (text.toDouble()!=0.)
  {
    this->material->setScalarPropertyGUI("CCX_DAMPING_USE_CARD",1);
    this->material->setScalarPropertyGUI("CCX_DAMPING_DAMPING",text.toDouble());
  } else {
    this->material->setScalarPropertyGUI("CCX_DAMPING_USE_CARD",0);
    this->material->setScalarPropertyGUI("CCX_DAMPING_DAMPING",0);
  }
}