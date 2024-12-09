#include "MaterialManagementHyperelasticCard.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementTable.hpp"

MaterialManagementHyperelasticCard::MaterialManagementHyperelasticCard(QWidget* parent, MaterialManagementItem *material):
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
  horizontal_spacer_type = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  label_title = new QLabel();
  label_title->setText("Hyperelastic Card");
  label_type = new QLabel();
  label_type->setText("Material Behavior");
  hyperelastic_type = new QComboBox();
  hyperelastic_type->addItem("ARRUDA-BOYCE");
  hyperelastic_type->addItem("MOONEY-RIVLIN");
  hyperelastic_type->addItem("NEO HOOKE");
  hyperelastic_type->addItem("OGDEN N=1");
  hyperelastic_type->addItem("OGDEN N=2");
  hyperelastic_type->addItem("OGDEN N=3");
  hyperelastic_type->addItem("POLYNOMIAL N=1");
  hyperelastic_type->addItem("POLYNOMIAL N=2");
  hyperelastic_type->addItem("POLYNOMIAL N=3");
  hyperelastic_type->addItem("REDUCED POLYNOMIAL N=1");
  hyperelastic_type->addItem("REDUCED POLYNOMIAL N=2");
  hyperelastic_type->addItem("REDUCED POLYNOMIAL N=3");
  hyperelastic_type->addItem("YEOH");

  table_arruda_boyce = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_ARRUDA_BOYCE");
  table_mooney_rivlin = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_MOONEY_RIVLIN");
  table_neo_hooke = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_NEO_HOOKE");
  table_ogden_n1 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_OGDEN_N1");
  table_ogden_n2 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_OGDEN_N2");
  table_ogden_n3 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_OGDEN_N3");
  table_polynomial_n1 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_POLYNOMIAL_N1");
  table_polynomial_n2 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_POLYNOMIAL_N2");
  table_polynomial_n3 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_POLYNOMIAL_N3");
  table_reduced_polynomial_n1 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1");
  table_reduced_polynomial_n2 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2");
  table_reduced_polynomial_n3 = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3");
  table_yeoh = new MaterialManagementTable(nullptr,"CCX_HYPERELASTIC_YEOH");
  
  // Layout
  GridLayout->addLayout(HBoxLayout,0,0, Qt::AlignTop);
  HBoxLayout->addLayout(VBoxLayout);
  HBoxLayout->addItem(horizontal_spacer);
  VBoxLayout->addWidget(label_title);
  HBoxLayout_type = new QHBoxLayout();
  HBoxLayout_type->addWidget(label_type);
  HBoxLayout_type->addWidget(hyperelastic_type);
  HBoxLayout_type->addItem(horizontal_spacer_type);
  VBoxLayout->addLayout(HBoxLayout_type);
  VBoxLayout->addWidget(table_arruda_boyce);
  VBoxLayout->addWidget(table_mooney_rivlin);
  VBoxLayout->addWidget(table_neo_hooke);
  VBoxLayout->addWidget(table_ogden_n1);
  VBoxLayout->addWidget(table_ogden_n2);
  VBoxLayout->addWidget(table_ogden_n3);
  VBoxLayout->addWidget(table_polynomial_n1);
  VBoxLayout->addWidget(table_polynomial_n2);
  VBoxLayout->addWidget(table_polynomial_n3);
  VBoxLayout->addWidget(table_reduced_polynomial_n1);
  VBoxLayout->addWidget(table_reduced_polynomial_n2);
  VBoxLayout->addWidget(table_reduced_polynomial_n3);
  VBoxLayout->addWidget(table_yeoh);
  VBoxLayout->addItem(vertical_spacer);

  QObject::connect(hyperelastic_type, SIGNAL(currentIndexChanged(int)),this,SLOT(hyperelastic_type_currentIndexChanged(int)));

  isInitialized = true;
}

MaterialManagementHyperelasticCard::~MaterialManagementHyperelasticCard()
{
}

void MaterialManagementHyperelasticCard::update(MaterialManagementItem *material)
{
  this->material = material;

  if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(0);
    table_arruda_boyce->show();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(1);
    table_arruda_boyce->hide();
    table_mooney_rivlin->show();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(2);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->show();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(3);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->show();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(4);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->show();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(5);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->show();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(6);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->show();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(7);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->show();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(8);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->show();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(9);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->show();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(10);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->show();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }else if (this->material->getScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD")==1)
  {
    hyperelastic_type->setCurrentIndex(12);
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->show();
  }else{
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 1);
    hyperelastic_type->setCurrentIndex(0);
    table_arruda_boyce->show();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
  }
  
  table_arruda_boyce->update(material);
  table_mooney_rivlin->update(material);
  table_neo_hooke->update(material);
  table_ogden_n1->update(material);
  table_ogden_n2->update(material);
  table_ogden_n3->update(material);
  table_polynomial_n1->update(material);
  table_polynomial_n2->update(material);
  table_polynomial_n3->update(material);
  table_reduced_polynomial_n1->update(material);
  table_reduced_polynomial_n2->update(material);
  table_reduced_polynomial_n3->update(material);
  table_yeoh->update(material);
}

void MaterialManagementHyperelasticCard::hyperelastic_type_currentIndexChanged(int index)
{
  if (index == 0)
  {
    table_arruda_boyce->show();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 1)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->show();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 2)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->show();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 3)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->show();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 4)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->show();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 5)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->show();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 6)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->show();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 7)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->show();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 8)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->show();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 9)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->show();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 10)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->show();
    table_reduced_polynomial_n3->hide();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 11)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->show();
    table_yeoh->hide();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 1);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 0);
  }else if (index == 12)
  {
    table_arruda_boyce->hide();
    table_mooney_rivlin->hide();
    table_neo_hooke->hide();
    table_ogden_n1->hide();
    table_ogden_n2->hide();
    table_ogden_n3->hide();
    table_polynomial_n1->hide();
    table_polynomial_n2->hide();
    table_polynomial_n3->hide();
    table_reduced_polynomial_n1->hide();
    table_reduced_polynomial_n2->hide();
    table_reduced_polynomial_n3->hide();
    table_yeoh->show();
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_ARRUDA_BOYCE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_MOONEY_RIVLIN_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_NEO_HOOKE_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_OGDEN_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3_USE_CARD", 0);
    this->material->setScalarPropertyGUI("CCX_HYPERELASTIC_YEOH_USE_CARD", 1);
  }
}