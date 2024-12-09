#include "MaterialManagement.hpp"
#include "CalculiXCoreInterface.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementElasticCard.hpp"
#include "MaterialManagementPlasticCard.hpp"
#include "MaterialManagementDensityCard.hpp"
#include "MaterialManagementSpecificHeatCard.hpp"
#include "MaterialManagementExpansionCard.hpp"
#include "MaterialManagementConductivityCard.hpp"
#include "MaterialManagementDeformationPlasticityCard.hpp"
#include "MaterialManagementMohrCoulombCard.hpp"
#include "MaterialManagementMohrCoulombHardeningCard.hpp"
#include "MaterialManagementCyclicHardeningCard.hpp"
#include "MaterialManagementRateDependentCard.hpp"
#include "MaterialManagementCreepCard.hpp"
#include "MaterialManagementHyperelasticCard.hpp"
#include "MaterialManagementHyperfoamCard.hpp"
#include "MaterialManagementCompressionOnlyCard.hpp"
#include "MaterialManagementTensionOnlyCard.hpp"
#include "MaterialManagementDampingCard.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"

MaterialManagement::MaterialManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  this->setGeometry(0,0,1520,800);
  this->setWindowTitle("Material Management");
  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_material1 = new QVBoxLayout();
  boxLayout_material2 = new QVBoxLayout();
  boxLayout_material_cards1 = new QVBoxLayout();
  boxLayout_material_cards2 = new QVBoxLayout();
  boxLayout_material_cards3 = new QVBoxLayout();
  boxLayout_materiallibrary1 = new QVBoxLayout();
  boxLayout_materiallibrary2 = new QVBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,2,6, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_material1,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material2,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_materiallibrary1,0,2, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_materiallibrary2,0,3, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material_cards1,0,4, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material_cards2,0,5, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material_cards3,0,6, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_widget,1,0,1,7, Qt::AlignTop);
  boxLayout_material2->addSpacing(30);
  boxLayout_material_cards2->addSpacing(30);
  boxLayout_materiallibrary2->addSpacing(30);

  // buttons
  pushButton_ok = new QPushButton();
  //pushButton_ok->setGeometry(310,530,75,23);
  pushButton_ok->setText("OK");
  boxLayout_window->addWidget(pushButton_ok);

  pushButton_apply = new QPushButton();
  //pushButton_apply->setGeometry(400,530,75,23);
  pushButton_apply->setText("Apply");
  boxLayout_window->addWidget(pushButton_apply);

  pushButton_close = new QPushButton();
  //pushButton_close->setGeometry(490,530,75,23);
  pushButton_close->setText("Close");
  boxLayout_window->addWidget(pushButton_close);
  
  pushButton_new = new QPushButton();
  //pushButton_new->setGeometry(200,30,75,23);
  pushButton_new->setText("New");
  boxLayout_material2->addWidget(pushButton_new);

  pushButton_delete = new QPushButton();
  //pushButton_delete->setGeometry(200,60,75,23);
  pushButton_delete->setText("Delete");
  boxLayout_material2->addWidget(pushButton_delete);
  
  pushButton_rename = new QPushButton();
  pushButton_rename->setText("Rename");
  boxLayout_material2->addWidget(pushButton_rename);

  pushButton_export = new QPushButton();
  pushButton_export->setText("Export");
  boxLayout_material2->addWidget(pushButton_export);

  pushButton_add = new QPushButton();
  //pushButton_add->setGeometry(420,70,21,21);
  pushButton_add->setText(">");
  boxLayout_material_cards2->addWidget(pushButton_add);

  pushButton_remove = new QPushButton();
  //pushButton_remove->setGeometry(420,100,21,21);
  pushButton_remove->setText("<");
  boxLayout_material_cards2->addWidget(pushButton_remove);

  //label_materiallibrary_material = new QLabel();
  //label_materiallibrary_material->setText("Material");
  //boxLayout_materiallibrary2->addWidget(label_materiallibrary_material);

  pushButton_library_import = new QPushButton();
  pushButton_library_import->setText("Import");
  boxLayout_materiallibrary2->addWidget(pushButton_library_import);

  pushButton_library_new = new QPushButton();
  pushButton_library_new->setText("New");
  boxLayout_materiallibrary2->addWidget(pushButton_library_new);

  pushButton_library_delete = new QPushButton();
  pushButton_library_delete->setText("Delete");
  boxLayout_materiallibrary2->addWidget(pushButton_library_delete);
  
  pushButton_library_rename = new QPushButton();
  pushButton_library_rename->setText("Rename");
  boxLayout_materiallibrary2->addWidget(pushButton_library_rename);

  label_materiallibrary_group = new QLabel();
  label_materiallibrary_group->setText("Group");
  boxLayout_materiallibrary2->addWidget(label_materiallibrary_group);

  pushButton_library_new_group = new QPushButton();
  pushButton_library_new_group->setText("New");
  boxLayout_materiallibrary2->addWidget(pushButton_library_new_group);

  pushButton_library_delete_group = new QPushButton();
  pushButton_library_delete_group->setText("Delete");
  boxLayout_materiallibrary2->addWidget(pushButton_library_delete_group);
  
  pushButton_library_rename_group = new QPushButton();
  pushButton_library_rename_group->setText("Rename");
  boxLayout_materiallibrary2->addWidget(pushButton_library_rename_group);

  label_materiallibrary_description = new QLabel();
  label_materiallibrary_description->setText("Description");
  boxLayout_materiallibrary2->addWidget(label_materiallibrary_description);

  pushButton_library_description = new QPushButton();
  pushButton_library_description->setText("Edit");
  boxLayout_materiallibrary2->addWidget(pushButton_library_description);

  // labels
  label_material = new QLabel();
  //label_material->setGeometry(10,10,80,16);
  label_material->setText("Material");
  boxLayout_material1->addWidget(label_material);

  label_available = new QLabel();
  //label_available->setGeometry(300,10,100,16);
  label_available->setText("Available Cards");
  boxLayout_material_cards1->addWidget(label_available);

  label_used = new QLabel();
  //label_used->setGeometry(450,10,100,16);
  label_used->setText("Used Cards");
  boxLayout_material_cards3->addWidget(label_used);

  label_materiallibrary = new QLabel();
  label_materiallibrary->setText("Library");
  boxLayout_materiallibrary1->addWidget(label_materiallibrary);

  // tree/lists
  tree_material = new QTreeWidget();
  //tree_material->setGeometry(10,30,181,191);
  tree_material->setColumnCount(2);
  tree_material->setColumnWidth(0, 200);
  tree_material->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_material1->addWidget(tree_material);

  list_available = new QListWidget();
  //list_available->setGeometry(300,30,111,191);
  boxLayout_material_cards1->addWidget(list_available);

  list_used = new QListWidget();
  //list_used->setGeometry(450,30,111,191);
  boxLayout_material_cards3->addWidget(list_used);

  tree_materiallibrary = new QTreeWidget();
  tree_materiallibrary->setColumnCount(2);
  tree_materiallibrary->setColumnWidth(0, 200);
  tree_materiallibrary->setHeaderLabels(QStringList() << "Name" << "Description");
  boxLayout_materiallibrary1->addWidget(tree_materiallibrary);

  // card widgets
  card_frame = new QFrame();
  //card_widget->setGeometry(10,10,500,250);
  card_frame->setMinimumSize(700,350);
  card_frame->setLineWidth(1);
  card_frame->setMidLineWidth(0);
  card_frame->setFrameStyle(QFrame::Box | QFrame::Raised);
  //card_widget->setStyleSheet("border: 1px solid black");
  
  elastic_widget = new MaterialManagementElasticCard(card_frame,current_material_item);
  plastic_widget = new MaterialManagementPlasticCard(card_frame,current_material_item);
  density_widget = new MaterialManagementDensityCard(card_frame,current_material_item);
  expansion_widget = new MaterialManagementExpansionCard(card_frame,current_material_item);
  specific_heat_widget = new MaterialManagementSpecificHeatCard(card_frame,current_material_item);
  conductivity_widget = new MaterialManagementConductivityCard(card_frame,current_material_item);
  deformation_plasticity_widget = new MaterialManagementDeformationPlasticityCard(card_frame,current_material_item);
  mohr_coulomb_widget = new MaterialManagementMohrCoulombCard(card_frame,current_material_item);
  mohr_coulomb_hardening_widget =  new MaterialManagementMohrCoulombHardeningCard(card_frame,current_material_item);
  cyclic_hardening_widget = new MaterialManagementCyclicHardeningCard(card_frame,current_material_item);
  rate_dependent_widget =  new MaterialManagementRateDependentCard(card_frame,current_material_item);
  creep_widget = new MaterialManagementCreepCard(card_frame,current_material_item);
  hyperelastic_widget = new MaterialManagementHyperelasticCard(card_frame,current_material_item);
  hyperfoam_widget = new MaterialManagementHyperfoamCard(card_frame,current_material_item);
  compression_only_widget = new MaterialManagementCompressionOnlyCard(card_frame,current_material_item);
  tension_only_widget = new MaterialManagementTensionOnlyCard(card_frame,current_material_item);
  damping_widget = new MaterialManagementDampingCard(card_frame,current_material_item);

  boxLayout_widget->addWidget(card_frame);

  //card_frame->show();
  elastic_widget->hide();
  plastic_widget->hide();
  density_widget->hide();
  expansion_widget->hide();
  specific_heat_widget->hide();
  conductivity_widget->hide();
  deformation_plasticity_widget->hide();
  mohr_coulomb_widget->hide();
  mohr_coulomb_hardening_widget->hide();
  cyclic_hardening_widget->hide();
  rate_dependent_widget->hide();
  creep_widget->hide();
  hyperelastic_widget->hide();
  hyperfoam_widget->hide();
  compression_only_widget->hide();
  tension_only_widget->hide();
  damping_widget->hide();

  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_new, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_new_clicked(bool)));
  QObject::connect(pushButton_delete, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_delete_clicked(bool)));
  QObject::connect(pushButton_rename, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_rename_clicked(bool)));
  QObject::connect(pushButton_export, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_export_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_remove_clicked(bool)));
  QObject::connect(pushButton_library_import, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_import_clicked(bool)));
  QObject::connect(pushButton_library_new, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_new_clicked(bool)));
  QObject::connect(pushButton_library_delete, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_delete_clicked(bool)));
  QObject::connect(pushButton_library_rename, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_rename_clicked(bool)));
  QObject::connect(pushButton_library_new_group, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_new_group_clicked(bool)));
  QObject::connect(pushButton_library_delete_group, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_delete_group_clicked(bool)));
  QObject::connect(pushButton_library_rename_group, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_rename_group_clicked(bool)));
  QObject::connect(pushButton_library_description, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_library_description_clicked(bool)));
  
  QObject::connect(tree_material, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,  SLOT(material_clicked(QTreeWidgetItem*, int)));
  QObject::connect(tree_material, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,  SLOT(material_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
  QObject::connect(tree_materiallibrary, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,  SLOT(library_material_clicked(QTreeWidgetItem*, int)));
  QObject::connect(tree_materiallibrary , SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,  SLOT(library_material_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
  QObject::connect(list_used, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(material_card_clicked(QListWidgetItem*)));
  QObject::connect(list_available, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(material_card_clicked(QListWidgetItem*)));
  QObject::connect(list_used, SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,  SLOT(material_card_doubleclicked(QListWidgetItem*)));
  QObject::connect(list_available, SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,  SLOT(material_card_doubleclicked(QListWidgetItem*)));
  QObject::connect(list_used, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(material_card_changed(QListWidgetItem*,QListWidgetItem*)));  
  QObject::connect(list_available, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(material_card_changed(QListWidgetItem*,QListWidgetItem*)));

  // Update list items and data
  this->update();
  current_material_item = nullptr;
}

MaterialManagement::~MaterialManagement()
{}

void MaterialManagement::clear()
{
  // Remove all of our tree items from material tree.
  /*while (tree_material->topLevelItemCount()>0)
  {
    tree_material->removeItemWidget(tree_material->topLevelItem(0),0);
  }
  this->removeListItems();*/
}

void MaterialManagement::update()
{
  this->update_cubit();
  this->update_materiallibrary();
}

void MaterialManagement::update_cubit()
{
  std::vector<std::vector<std::string>> material_tree_data;
  material_tree_data = ccx_iface->get_material_tree_data(); 

  MaterialManagementItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < material_tree_data.size(); i++)
  {
    // check if material already exists as item
    
    ChildId = this->get_child_id(material_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addMaterial(QString::fromStdString(material_tree_data[i][0]), QString::fromStdString(material_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(ChildId));
      
      str_check = material_tree_data[i][1];    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
        temp_child->material_name_qstring = QString::fromStdString(str_check);
        temp_child->material_name = str_check;
      }
    }
  }

  // check if item has been removed

  for (size_t i = tree_material->topLevelItemCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(int(i)-1));

    for (size_t ii = 0; ii < material_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==material_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      this->removeMaterial(temp_child);
    }
  }

  // update childs
  for (size_t i = tree_material->topLevelItemCount(); i > 0; i--)
  { 
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(int(i)-1));
    temp_child->update();
  }

  tree_material->sortItems(1,Qt::AscendingOrder);
}

void MaterialManagement::update_materiallibrary()
{
  QList<QTreeWidgetItem*> items;
  MaterialManagementItem *temp_child;
  bool erase_item;

  std::vector<std::vector<std::string>> materiallibrary_tree_data;
  materiallibrary_tree_data = ccx_iface->get_materiallibrary_tree_data();

  for (size_t i = 0; i < materiallibrary_tree_data.size(); i++)
  {
    // check if item already exists
    if (!this->check_library_item_exists(materiallibrary_tree_data[i][2]))
    {
      this->addMateriallibrary(materiallibrary_tree_data[i]);
    }
    // modify description if necessary
    temp_child = this->get_library_item(materiallibrary_tree_data[i][2]);
    
    if (temp_child != nullptr)
    {
      std::string str_check = materiallibrary_tree_data[i][1];
      if (temp_child->text(1).toStdString() != str_check)
      {
        temp_child->setText(1, QString::fromStdString(str_check));
        temp_child->library_description_qstring = QString::fromStdString(str_check);
        temp_child->library_description = str_check;
      }
    }
  }

  // check if item has been removed
  items = tree_materiallibrary->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
  for (size_t i = items.count(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = dynamic_cast<MaterialManagementItem*>(items.at(int(i)-1));
    
    for (size_t ii = 0; ii < materiallibrary_tree_data.size(); ii++)
      {    
        if (temp_child->hdf5path==materiallibrary_tree_data[ii][2])
        {
          erase_item = false;
          break;
        }
      }

      if (erase_item)
      {
        this->removeMateriallibrary(temp_child);
      }
  }

  // update childs
  items = tree_materiallibrary->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);
  for (size_t i = 0; i < items.count(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(items.at(int(i)));
    temp_child->update();
  }

  tree_materiallibrary->sortItems(0,Qt::AscendingOrder);
}

void MaterialManagement::addMaterial(QString material_id, QString material_name)
{
  MaterialManagementItem *MaterialTreeChild = new MaterialManagementItem(tree_material);
  MaterialTreeChild->initialize_cubit(material_id, material_name);
  MaterialTreeChild->setIcon(0,ccx_iface->getIcon2("MaterialTree"));
}

void MaterialManagement::removeMaterial(MaterialManagementItem *material)
{
  tree_material->removeItemWidget(material,0);
  delete material;
}

void MaterialManagement::printproperties()
{
  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(int(i)));
    
    log = "########### \n";
    log.append(temp_child->text(1).toStdString() + " child id \n");
    for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
    {
      log.append("----------\n");
      log.append(temp_child->group_properties[temp_child->properties[ii][0]][0] + " \n");
      log.append("----------\n");
      log.append(std::to_string(temp_child->properties[ii][0]) + " " + std::to_string(temp_child->properties[ii][1]) + " " + std::to_string(temp_child->properties[ii][2]) + " \n");
      if (temp_child->properties[ii][1]==1)
      {
        log.append("SCALAR\n");
        log.append(std::to_string(temp_child->property_scalar[temp_child->properties[ii][2]]) + " \n");
        log.append("SCALAR GUI\n");
        log.append(std::to_string(temp_child->property_scalar_gui[temp_child->properties[ii][2]]) + " \n");
      } else if (temp_child->properties[ii][1]==4)
      {        
        log.append("MATRIX\n");
        for (size_t iii = 0; iii < temp_child->property_matrix[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
        log.append("MATRIX GUI\n");
        for (size_t iii = 0; iii < temp_child->property_matrix_gui[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
      }
    }
    PRINT_INFO("%s", log.c_str());
  }
}

int MaterialManagement::get_child_id(std::string material_id)
{
  int int_return;
  int_return = -1;

  //log = std::to_string(tree_material->topLevelItemCount()) + " child count \n";
  //PRINT_INFO("%s", log.c_str());

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(int(i)));
    
    //log = temp_child->text(1).toStdString() + " == " + material_id + " child ids \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->text(1).toStdString()==material_id)
    {
      int_return = int(i);
      break;
    }
  }
  return int_return;
}

bool MaterialManagement::check_library_item_exists(std::string path)
{
  QList<QTreeWidgetItem*> items = tree_materiallibrary->findItems(
            QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < items.count(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(items.at(int(i)));

    //log = temp_child->hdf5path + " == " + path + " \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->hdf5path == path)
    {
      return true;
    }
  }
 
  return false;
}

MaterialManagementItem* MaterialManagement::get_library_item(std::string path)
{
  QList<QTreeWidgetItem*> items = tree_materiallibrary->findItems(
            QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < items.count(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(items.at(int(i)));
    
    //log = temp_child->hdf5path + " == " + path + " \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->hdf5path == path)
    {
      return temp_child;
    }
  }
 
  return nullptr;
}

void MaterialManagement::addMateriallibrary(std::vector<std::string> tree_data)
{
  MaterialManagementItem *MaterialTreeChild;
  MaterialManagementItem *parent = this->get_library_item(tree_data[2].substr(0,tree_data[2].length()-tree_data[0].length()-1));

  if (parent!=nullptr)
  {
    MaterialTreeChild = new MaterialManagementItem(parent);
  }else{
    MaterialTreeChild = new MaterialManagementItem(tree_materiallibrary);
  }
    
  MaterialTreeChild->initialize_library(tree_data);
  if (tree_data[3] == "Material")
  {
    MaterialTreeChild->setIcon(0,ccx_iface->getIcon2("MaterialTree"));
  }
  if (tree_data[3] == "Group")
  {
    MaterialTreeChild->setIcon(0,ccx_iface->getIcon2("MaterialLibraryGroup"));
  }
}

void MaterialManagement::removeMateriallibrary(MaterialManagementItem *item)
{
  tree_materiallibrary->removeItemWidget(item,0);
  delete item;
}

void MaterialManagement::createListItems(MaterialManagementItem *material)
{
  bool use_elastic = false;
  bool use_plastic = false;
  bool use_density = false;
  bool use_specific_heat = false;
  bool use_expansion = false;
  bool use_conductivity = false;
  bool use_deformation_plasticity = false;
  bool use_mohr_coulomb = false;
  bool use_mohr_coulomb_hardening = false;
  bool use_cylclic_hardening = false;
  bool use_rate_dependent = false;
  bool use_creep = false;
  bool use_hyperelastic = false;
  bool use_hyperfoam = false;
  bool use_compression_only = false;
  bool use_tension_only = false;
  bool use_damping = false;
  
  for (size_t i = 0; i < material->properties.size(); i++)
  {
    //log = std::to_string(material->properties[i].size()) + " \n";
    //PRINT_INFO("%s", log.c_str());
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_ELASTIC_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_elastic = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_PLASTIC_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_plastic = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_DENSITY_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_density = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_SPECIFIC_HEAT_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_specific_heat = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_EXPANSION_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_expansion = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_CONDUCTIVITY_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_conductivity = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_DEFORMATION_PLASTICITY_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_deformation_plasticity = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_MOHR_COULOMB_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_mohr_coulomb = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_MOHR_COULOMB_HARDENING_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_mohr_coulomb_hardening = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_CYCLIC_HARDENING_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_cylclic_hardening = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_RATE_DEPENDENT_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_rate_dependent = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_CREEP_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_creep = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_HYPERELASTIC_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_hyperelastic = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_HYPERFOAM_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_hyperfoam = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_COMPRESSION_ONLY_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_compression_only = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_TENSION_ONLY_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_tension_only = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_DAMPING_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_damping = true;
      }
    }
  }

  if (use_elastic)
  {
    list_elastic = new QListWidgetItem("Elastic",list_used);
  } else {
    list_elastic = new QListWidgetItem("Elastic",list_available);
  }
  
  if (use_plastic)
  {
    list_plastic = new QListWidgetItem("Plastic",list_used);
  } else {
    list_plastic = new QListWidgetItem("Plastic",list_available);
  }

  if (use_density)
  {
    list_density = new QListWidgetItem("Density",list_used);
  } else {
    list_density = new QListWidgetItem("Density",list_available);
  }

  if (use_specific_heat)
  {
    list_specific_heat = new QListWidgetItem("Specific heat",list_used);
  } else {
    list_specific_heat = new QListWidgetItem("Specific heat",list_available);
  }

  if (use_expansion)
  {
    list_expansion = new QListWidgetItem("Expansion",list_used);
  } else {
    list_expansion = new QListWidgetItem("Expansion",list_available);
  }
  
  if (use_conductivity)
  {
    list_conductivity = new QListWidgetItem("Conductivity",list_used);
  } else {
    list_conductivity = new QListWidgetItem("Conductivity",list_available);
  }

  if (use_deformation_plasticity)
  {
    list_deformation_plasticity = new QListWidgetItem("Deformation Plasticity",list_used);
  } else {
    list_deformation_plasticity = new QListWidgetItem("Deformation Plasticity",list_available);
  }
  
  if (use_mohr_coulomb)
  {
    list_mohr_coulomb = new QListWidgetItem("Mohr Coulomb",list_used);
  } else {
    list_mohr_coulomb = new QListWidgetItem("Mohr Coulomb",list_available);
  }

  if (use_mohr_coulomb_hardening)
  {
    list_mohr_coulomb_hardening = new QListWidgetItem("Mohr Coulomb Hardening",list_used);
  } else {
    list_mohr_coulomb_hardening = new QListWidgetItem("Mohr Coulomb Hardening",list_available);
  }

  if (use_cylclic_hardening)
  {
    list_cylclic_hardening = new QListWidgetItem("Cyclic Hardening",list_used);
  } else {
    list_cylclic_hardening = new QListWidgetItem("Cyclic Hardening",list_available);
  }

  if (use_rate_dependent)
  {
    list_rate_dependent = new QListWidgetItem("Rate Dependent",list_used);
  } else {
    list_rate_dependent = new QListWidgetItem("Rate Dependent",list_available);
  }

  if (use_creep)
  {
    list_creep = new QListWidgetItem("Creep",list_used);
  } else {
    list_creep = new QListWidgetItem("Creep",list_available);
  }

  if (use_hyperelastic)
  {
    list_hyperelastic = new QListWidgetItem("Hyperelastic",list_used);
  } else {
    list_hyperelastic = new QListWidgetItem("Hyperelastic",list_available);
  }

  if (use_hyperfoam)
  {
    list_hyperfoam = new QListWidgetItem("Hyperfoam",list_used);
  } else {
    list_hyperfoam = new QListWidgetItem("Hyperfoam",list_available);
  }

  if (use_compression_only)
  {
    list_compression_only = new QListWidgetItem("Compression Only",list_used);
  } else {
    list_compression_only = new QListWidgetItem("Compression Only",list_available);
  }

  if (use_tension_only)
  {
    list_tension_only = new QListWidgetItem("Tension Only",list_used);
  } else {
    list_tension_only = new QListWidgetItem("Tension Only",list_available);
  }

  if (use_damping)
  {
    list_damping = new QListWidgetItem("Damping",list_used);
  } else {
    list_damping = new QListWidgetItem("Damping",list_available);
  }

  list_available->sortItems();
  list_used->sortItems();
}

void MaterialManagement::removeListItems()
{
  if (list_available->count()>0)
  {
    list_available->setCurrentItem(list_available->item(0));
    while (list_available->currentItem())
    {
      delete list_available->currentItem();
    }
  } 

  if (list_used->count()>0)
  {
    list_used->setCurrentItem(list_used->item(0));
    while (list_used->currentItem())
    {
      delete list_used->currentItem();
    }
  } 
}

void MaterialManagement::switchListItem(QListWidget* source, QListWidget* target)
{
  if(source->currentItem()){
    QListWidgetItem* newItem = source->currentItem()->clone();
    target->addItem(newItem);
    target->setCurrentItem(newItem);
    delete source->currentItem();
    this->selectListItem(newItem);
    this->loadWidget(newItem);
    source->sortItems();
    target->sortItems();

    if (newItem->text()=="Elastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_ELASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_ELASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_ELASTIC_USE_CARD", 0);
      }
    }else if (newItem->text()=="Plastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_PLASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_PLASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_PLASTIC_USE_CARD", 0);
      }
    }else if (newItem->text()=="Density")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_DENSITY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_DENSITY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_DENSITY_USE_CARD", 0);
      }
    }else if (newItem->text()=="Specific heat")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_SPECIFIC_HEAT_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_SPECIFIC_HEAT_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_SPECIFIC_HEAT_USE_CARD", 0);
      }
    }else if (newItem->text()=="Expansion")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_EXPANSION_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_EXPANSION_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_EXPANSION_USE_CARD", 0);
      }
    }else if (newItem->text()=="Conductivity")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_CONDUCTIVITY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_CONDUCTIVITY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_CONDUCTIVITY_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Deformation Plasticity")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_DEFORMATION_PLASTICITY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_DEFORMATION_PLASTICITY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_DEFORMATION_PLASTICITY_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Mohr Coulomb")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_MOHR_COULOMB_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_MOHR_COULOMB_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_MOHR_COULOMB_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Mohr Coulomb Hardening")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_MOHR_COULOMB_HARDENING_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_MOHR_COULOMB_HARDENING_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_MOHR_COULOMB_HARDENING_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Cyclic Hardening")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_CYCLIC_HARDENING_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_CYCLIC_HARDENING_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_CYCLIC_HARDENING_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Rate Dependent")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_RATE_DEPENDENT_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_RATE_DEPENDENT_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_RATE_DEPENDENT_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Creep")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_CREEP_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_CREEP_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_CREEP_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Hyperelastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_HYPERELASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_HYPERELASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_HYPERELASTIC_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Hyperfoam")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_HYPERFOAM_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_HYPERFOAM_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_HYPERFOAM_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Compression Only")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_COMPRESSION_ONLY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_COMPRESSION_ONLY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_COMPRESSION_ONLY_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Tension Only")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_TENSION_ONLY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_TENSION_ONLY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_TENSION_ONLY_USE_CARD", 0);
      } 
    }else if (newItem->text()=="Damping")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_DAMPING_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_DAMPING_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_DAMPING_USE_CARD", 0);
      } 
    }
  }
}

void MaterialManagement::selectListItem(QListWidgetItem* item)
{
  QListWidgetItem* temp_item;

  for (size_t i = 0; i < list_available->count(); i++)
  {
    temp_item = list_available->item(int(i));
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }
  for (size_t i = 0; i < list_used->count(); i++)
  {
    temp_item = list_used->item(int(i));
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }  
}

void MaterialManagement::loadWidget(QListWidgetItem* item)
{
  elastic_widget->hide();
  plastic_widget->hide();
  density_widget->hide();
  specific_heat_widget->hide();
  expansion_widget->hide();
  conductivity_widget->hide();
  deformation_plasticity_widget->hide();
  mohr_coulomb_widget->hide();
  mohr_coulomb_hardening_widget->hide();
  cyclic_hardening_widget->hide();
  rate_dependent_widget->hide();
  creep_widget->hide();
  hyperelastic_widget->hide();
  hyperfoam_widget->hide();
  compression_only_widget->hide();
  tension_only_widget->hide();
  damping_widget->hide();

  if (item->text().toStdString()=="Elastic")
  {
    elastic_widget->show();
  }else if (item->text().toStdString()=="Plastic")
  {
    plastic_widget->show();
  }else if (item->text().toStdString()=="Density")
  {
    density_widget->show();
  }else if (item->text().toStdString()=="Expansion")
  {
    expansion_widget->show();
  }else if (item->text().toStdString()=="Specific heat")
  {
    specific_heat_widget->show();
  }else if (item->text().toStdString()=="Conductivity")
  {
    conductivity_widget->show();
  }else if (item->text().toStdString()=="Deformation Plasticity")
  {
    deformation_plasticity_widget->show();
  }else if (item->text().toStdString()=="Mohr Coulomb")
  {
    mohr_coulomb_widget->show();
  }else if (item->text().toStdString()=="Mohr Coulomb Hardening")
  {
    mohr_coulomb_hardening_widget->show();
  }else if (item->text().toStdString()=="Cyclic Hardening")
  {
    cyclic_hardening_widget->show();
  }else if (item->text().toStdString()=="Rate Dependent")
  {
    rate_dependent_widget->show();
  }else if (item->text().toStdString()=="Creep")
  {
    creep_widget->show();
  }else if (item->text().toStdString()=="Hyperelastic")
  {
    hyperelastic_widget->show();
  }else if (item->text().toStdString()=="Hyperfoam")
  {
    hyperfoam_widget->show();
  }else if (item->text().toStdString()=="Compression Only")
  {
    compression_only_widget->show();
  }else if (item->text().toStdString()=="Tension Only")
  {
    tension_only_widget->show();
  }else if (item->text().toStdString()=="Damping")
  {
    damping_widget->show();
  }
}

void MaterialManagement::setMaterial(QString material_id)
{
  MaterialManagementItem* current_item;
  int ChildId;
  ChildId = get_child_id(material_id.toStdString());
  if (ChildId!=-1)
  {
    current_item = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(ChildId));
    if (current_item!=NULL)
    {
      tree_material->setCurrentItem(current_item);
    }
  }
}

void MaterialManagement::on_pushButton_ok_clicked(bool)
{
  //log = " clicked ok \n";
  //PRINT_INFO("%s", log.c_str());
  this->on_pushButton_apply_clicked(true);
  this->on_pushButton_close_clicked(true);
}

void MaterialManagement::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  //this->printproperties();
  QString command;
  QString command_prefix;

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(int(i)));
    command_prefix = "modify material \"" + temp_child->material_name_qstring + "\" ";

    for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
    {
      command = "";

      if ((temp_child->properties[ii][1]==1) &&
          (temp_child->property_scalar[temp_child->properties[ii][2]] != temp_child->property_scalar_gui[temp_child->properties[ii][2]]))
      {
        command.append(command_prefix);
        // scalar
        command.append("scalar_properties ");
        // group property
        command.append("\"" + QString::fromStdString(temp_child->group_properties[temp_child->properties[ii][0]][0]) + "\" ");
        // value
        command.append(QString::number(temp_child->property_scalar_gui[temp_child->properties[ii][2]]));
      } else if ((temp_child->properties[ii][1]==4) &&
          (temp_child->property_matrix[temp_child->properties[ii][2]] != temp_child->property_matrix_gui[temp_child->properties[ii][2]]))
      { 
        command.append(command_prefix);
        // matrix
        command.append("matrix_property ");
        // group property
        command.append("\"" + QString::fromStdString(temp_child->group_properties[temp_child->properties[ii][0]][0]) + "\" ");
        // value
        for (size_t iii = 0; iii < temp_child->property_matrix_gui[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            command.append(QString::number(temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
        }
      }
      if (command != "")
      {
        commands.push_back(command);
      }
    }
  }

  QList<QTreeWidgetItem*> items = tree_materiallibrary->findItems(QString("*"), Qt::MatchWrap | Qt::MatchWildcard | Qt::MatchRecursive);

  for (size_t i = 0; i < items.count(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(items.at(int(i)));
    if (temp_child->isLibraryMaterial)
    {
      command_prefix = "ccx modify materiallibrary material name \"" + temp_child->library_name_qstring + "\" groupname \"" + temp_child->library_group_qstring + "\" ";

      for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
      {
        command = "";

        if ((temp_child->properties[ii][1]==1) &&
            (temp_child->property_scalar[temp_child->properties[ii][2]] != temp_child->property_scalar_gui[temp_child->properties[ii][2]]))
        {
          command.append(command_prefix);
          // scalar
          command.append("scalar_properties ");
          // group property
          command.append("\"" + QString::fromStdString(temp_child->group_properties[temp_child->properties[ii][0]][0]) + "\" ");
          // value
          command.append(QString::number(temp_child->property_scalar_gui[temp_child->properties[ii][2]]));
        } else if ((temp_child->properties[ii][1]==4) &&
            (temp_child->property_matrix[temp_child->properties[ii][2]] != temp_child->property_matrix_gui[temp_child->properties[ii][2]]))
        { 
          command.append(command_prefix);
          // matrix
          command.append("matrix_property ");
          // group property
          command.append("\"" + QString::fromStdString(temp_child->group_properties[temp_child->properties[ii][0]][0]) + "\" ");
          // value
          for (size_t iii = 0; iii < temp_child->property_matrix_gui[temp_child->properties[ii][2]].size(); iii++)
          {
            for (size_t iv = 0; iv < temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii].size(); iv++)
            {
              command.append(QString::number(temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii][iv]) + " ");
            }
          }
        }
        if (command != "")
        {
          commands.push_back(command);
        }
      }
    }
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}

void MaterialManagement::on_pushButton_close_clicked(bool)
{
  //log = " clicked close \n";
  //PRINT_INFO("%s", log.c_str());
  this->close();
}

void MaterialManagement::on_pushButton_new_clicked(bool)
{
  //log = " clicked new \n";
  //PRINT_INFO("%s", log.c_str());
  
  //QStringList commands;
  std::vector<std::string> commands;
  bool ok;  
  QString name = QInputDialog::getText(0, "Create Material",
                                         "Material Name:", QLineEdit::Normal,
                                         "", &ok);
  if (ok && !name.isEmpty()){
    //commands.push_back("create material \"" + name + "\" property_group \"CalculiX-FEA\"");
    //commands.push_back();
    commands.push_back("create material \"" + name.toStdString() + "\" property_group \"CalculiX-FEA\"");
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[i].c_str());
    ccx_iface->cmd(commands[int(i)]);
  }
}

void MaterialManagement::on_pushButton_delete_clicked(bool)
{
  //log = " clicked delete \n";
  //PRINT_INFO("%s", log.c_str());
  //QStringList commands;
  
  if (tree_material->topLevelItemCount()==0)
  {
    return;
  }
  
  std::vector<std::string> commands;
  
  QMessageBox::StandardButton msgbox;
  msgbox = QMessageBox::question(this,"Delete Material","Delete Material " + current_material_item->material_name_qstring + "?", QMessageBox::Yes | QMessageBox::No);
  
  if (msgbox == QMessageBox::Yes)
  {
    //commands.push_back("delete material " + current_material_item->material_id_qstring);
    commands.push_back("delete material " + current_material_item->material_id_qstring.toStdString());
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[i].c_str());
    ccx_iface->cmd(commands[int(i)]);
  }

  this->removeListItems();
  
  elastic_widget->hide();
  plastic_widget->hide();
  density_widget->hide();
  specific_heat_widget->hide();
  expansion_widget->hide();
  conductivity_widget->hide();
  deformation_plasticity_widget->hide();
  mohr_coulomb_widget->hide();
  mohr_coulomb_hardening_widget->hide();
  cyclic_hardening_widget->hide();
  rate_dependent_widget->hide();
  creep_widget->hide();
  hyperelastic_widget->hide();
  hyperfoam_widget->hide();
  compression_only_widget->hide();
  tension_only_widget->hide();
  damping_widget->hide();
  
}

void MaterialManagement::on_pushButton_rename_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && current_material_item->isCubit)
  { 
    QString text = QInputDialog::getText(this, tr("Rename Material"),tr("Type new Name "), QLineEdit::Normal, current_material_item->material_name_qstring, &ok);
    if (ok && !text.isEmpty())
    {
      commands.push_back("modify material \"" + current_material_item->material_name_qstring.toStdString() + "\" name \"" + text.toStdString() + "\"" );
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_export_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  std::vector<std::vector<std::string>> materiallibrary_tree_data;
  QStringList items;
  //items.append(QString::fromStdString("/"));

  materiallibrary_tree_data = ccx_iface->get_materiallibrary_tree_data();
  for (size_t i = 0; i < materiallibrary_tree_data.size(); i++)
  {
    // check if item already exists
    if (materiallibrary_tree_data[i][3]=="Group")
    {
      items.append(QString::fromStdString(materiallibrary_tree_data[i][2]));
    }
  }
  
  if ((current_material_item != nullptr) && current_material_item->isCubit)
  { 
    QString text = "Export Cubit Material \n\n" + current_material_item->material_name_qstring + "\n\n into Library Group ";
    QString item = QInputDialog::getItem(this, tr("Export Material to Library"),text, items, 0 , false, &ok);
    if (ok && !item.isEmpty())
    {
      commands.push_back("ccx export materiallibrary cubit_name \"" + current_material_item->material_name_qstring.toStdString() + "\" name \"" + current_material_item->material_name_qstring.toStdString() + "\" groupname \"" + item.toStdString() + "\"" );
    } 

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_add_clicked(bool)
{
  //log = " clicked add \n";
  //PRINT_INFO("%s", log.c_str());
  switchListItem(list_available, list_used);
}

void MaterialManagement::on_pushButton_remove_clicked(bool)
{
  //log = " clicked remove \n";
  //PRINT_INFO("%s", log.c_str());
  switchListItem(list_used, list_available);
}

void MaterialManagement::on_pushButton_library_import_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && current_material_item->isLibraryMaterial)
  { 
    QString text = QInputDialog::getText(this, tr("Import Library Material"),tr("import with Name "), QLineEdit::Normal, current_material_item->library_name_qstring, &ok);
    if (ok && !text.isEmpty())
    {
      commands.push_back("ccx import materiallibrary name \"" + current_material_item->library_name + "\" groupname \"" + current_material_item->library_group + "\" cubit_name \"" + text.toStdString() + "\"" );
    } 

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_library_new_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && !current_material_item->isCubit)
  { 
    QString text = QInputDialog::getText(this, tr("Create Library Material"),tr("Type Name "), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
      if (current_material_item->isLibraryGroup)
      {
        commands.push_back("ccx create materiallibrary material name \"" + text.toStdString() + "\" groupname \"" + current_material_item->hdf5path + "\"" );
      }
      if (current_material_item->isLibraryMaterial)
      {
        commands.push_back("ccx create materiallibrary material name \"" + text.toStdString() + "\" groupname \"" + current_material_item->library_group + "\"" );
      }
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
  //root material, when nothing is selected
  if ((current_material_item == nullptr))
  { 
    QString text = QInputDialog::getText(this, tr("Create Library Material"),tr("Type Name "), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
      commands.push_back("ccx create materiallibrary material name \"" + text.toStdString() + "\" groupname \"/\"" );
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_library_delete_clicked(bool)
{
  std::vector<std::string> commands;
  if ((current_material_item != nullptr) && current_material_item->isLibraryMaterial)
  {
    QMessageBox::StandardButton msgbox;
    msgbox = QMessageBox::question(this,"Delete Library Material","Delete Material " + current_material_item->library_name_qstring + "?", QMessageBox::Yes | QMessageBox::No);
    
    if (msgbox == QMessageBox::Yes)
    {
      commands.push_back("ccx delete materiallibrary material name \"" + current_material_item->library_name +"\" groupname \"" + current_material_item->library_group +"\"");
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }

    this->removeListItems();
    
    elastic_widget->hide();
    plastic_widget->hide();
    density_widget->hide();
    specific_heat_widget->hide();
    expansion_widget->hide();
    conductivity_widget->hide();
    deformation_plasticity_widget->hide();
    mohr_coulomb_widget->hide();
    mohr_coulomb_hardening_widget->hide();
    cyclic_hardening_widget->hide();
    rate_dependent_widget->hide();
    creep_widget->hide();
    hyperelastic_widget->hide();
    hyperfoam_widget->hide();
    compression_only_widget->hide();
    tension_only_widget->hide();
    damping_widget->hide();

  }  
}

void MaterialManagement::on_pushButton_library_rename_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && current_material_item->isLibraryMaterial)
  { 
    QString text = QInputDialog::getText(this, tr("Rename Library Material"),tr("Type new Name "), QLineEdit::Normal, current_material_item->library_name_qstring, &ok);
    if (ok && !text.isEmpty())
    {
      commands.push_back("ccx modify materiallibrary material name \"" + current_material_item->library_name + "\" groupname \"" + current_material_item->library_group + "\" new_name \"" + text.toStdString() + "\"" );
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_library_new_group_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && !current_material_item->isCubit)
  { 
    QString text = QInputDialog::getText(this, tr("Create Library Group"),tr("Type Name "), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
      if (current_material_item->isLibraryGroup)
      {
        commands.push_back("ccx create materiallibrary group name \"" + current_material_item->hdf5path + "/" + text.toStdString() + "\"" );
      }
      if (current_material_item->isLibraryMaterial)
      {
        commands.push_back("ccx create materiallibrary group name \"" + current_material_item->library_group + "/" + text.toStdString() + "\"" );
      }
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
  //root material, when nothing is selected
  if ((current_material_item == nullptr))
  { 
    QString text = QInputDialog::getText(this, tr("Create Library Group"),tr("Type Name "), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
      commands.push_back("ccx create materiallibrary group name \"" + text.toStdString() + "\"" );
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_library_delete_group_clicked(bool)
{
  std::vector<std::string> commands;
  if ((current_material_item != nullptr) && current_material_item->isLibraryGroup)
  {
    QMessageBox::StandardButton msgbox;
    msgbox = QMessageBox::question(this,"Delete Library Group","Delete Group " + current_material_item->library_name_qstring + "?", QMessageBox::Yes | QMessageBox::No);
    
    if (msgbox == QMessageBox::Yes)
    {
      commands.push_back("ccx delete materiallibrary group name \"" + current_material_item->hdf5path +"\"");
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }

    this->removeListItems();
    
    elastic_widget->hide();
    plastic_widget->hide();
    density_widget->hide();
    specific_heat_widget->hide();
    expansion_widget->hide();
    conductivity_widget->hide();
    deformation_plasticity_widget->hide();
    mohr_coulomb_widget->hide();
    mohr_coulomb_hardening_widget->hide();
    cyclic_hardening_widget->hide();
    rate_dependent_widget->hide();
    creep_widget->hide();
    hyperelastic_widget->hide();
    hyperfoam_widget->hide();
    compression_only_widget->hide();
    tension_only_widget->hide();
    damping_widget->hide();

  }
}

void MaterialManagement::on_pushButton_library_rename_group_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && current_material_item->isLibraryGroup)
  { 
    QString text = QInputDialog::getText(this, tr("Rename Library Group"),tr("Type new Name "), QLineEdit::Normal, current_material_item->library_name_qstring, &ok);
    if (ok && !text.isEmpty())
    {
      commands.push_back("ccx modify materiallibrary group name \"" + current_material_item->hdf5path + "\" new_name \"" + current_material_item->library_group + "/" + text.toStdString() + "\"" );
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::on_pushButton_library_description_clicked(bool)
{
  std::vector<std::string> commands;
  bool ok = false;
  
  if ((current_material_item != nullptr) && !current_material_item->isCubit)
  { 
    QString text = QInputDialog::getText(this, tr("Edit Library Description"),tr("Edit Description "), QLineEdit::Normal, current_material_item->library_description_qstring, &ok);
    if (ok && !text.isEmpty())
    {
      if (current_material_item->isLibraryGroup)
      {
        commands.push_back("ccx modify materiallibrary group name \"" + current_material_item->hdf5path + "\" description \"" + text.toStdString() + "\"" );
      }
      if (current_material_item->isLibraryMaterial)
      {
        commands.push_back("ccx modify materiallibrary material name \"" + current_material_item->library_name + "\" groupname \"" + current_material_item->library_group + "\" description \"" + text.toStdString() + "\"" );
      }
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
      ccx_iface->cmd(commands[int(i)]);
    }
  }
}

void MaterialManagement::material_clicked(QTreeWidgetItem* item, int column)
{
  //log = " material clicked \n";
  //PRINT_INFO("%s", log.c_str());
  if (item==nullptr)
  {
    return;
  }

  MaterialManagementItem* material_item;
  
  if (material_item = dynamic_cast<MaterialManagementItem*>(item))
  {
    current_material_item = material_item;
    this->removeListItems();
    if ((current_material_item!=nullptr) && !current_material_item->isLibraryGroup)
    {
      this->createListItems(material_item);
    }

    elastic_widget->hide();
    plastic_widget->hide();
    density_widget->hide();
    specific_heat_widget->hide();
    expansion_widget->hide();
    conductivity_widget->hide();
    deformation_plasticity_widget->hide();
    mohr_coulomb_widget->hide();
    mohr_coulomb_hardening_widget->hide();
    cyclic_hardening_widget->hide();
    rate_dependent_widget->hide();
    creep_widget->hide();
    hyperelastic_widget->hide();
    hyperfoam_widget->hide();
    compression_only_widget->hide();
    tension_only_widget->hide();
    damping_widget->hide();


    if ((current_material_item!=nullptr) && !current_material_item->isLibraryGroup)
    {
      elastic_widget->update(material_item);
      plastic_widget->update(material_item);
      density_widget->update(material_item);
      specific_heat_widget->update(material_item);
      expansion_widget->update(material_item);
      conductivity_widget->update(material_item);
      deformation_plasticity_widget->update(material_item);
      mohr_coulomb_widget->update(material_item);
      mohr_coulomb_hardening_widget->update(material_item);
      cyclic_hardening_widget->update(material_item);
      rate_dependent_widget->update(material_item);
      creep_widget->update(material_item);
      hyperelastic_widget->update(material_item);
      hyperfoam_widget->update(material_item);
      compression_only_widget->update(material_item);
      tension_only_widget->update(material_item);
      damping_widget->update(material_item);
    }

    if (current_material_item->isCubit)
    {
      tree_materiallibrary->clearSelection();
    }
    if (current_material_item->isLibraryMaterial || current_material_item->isLibraryGroup)
    {
      tree_material->clearSelection();
    }
  }
}

void MaterialManagement::material_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->material_clicked(current_item,0);
  }
}

void MaterialManagement::material_card_clicked(QListWidgetItem* item)
{
  //log = " material card " + item->text().toStdString() + " clicked \n";
  //PRINT_INFO("%s", log.c_str()); 
  this->selectListItem(item);
  this->loadWidget(item);
}

void MaterialManagement::library_material_clicked(QTreeWidgetItem* item, int column)
{
  //log = " library item clicked \n";
  //PRINT_INFO("%s", log.c_str());

  if (item!=nullptr)
  {
    this->material_clicked(item,0);
  }
}

void MaterialManagement::library_material_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->library_material_clicked(current_item,0);
  }
}

void MaterialManagement::material_card_doubleclicked(QListWidgetItem* item)
{
  //log = " material card " + item->text().toStdString() + " clicked \n";
  //PRINT_INFO("%s", log.c_str()); 
  if (item->listWidget()==list_used)
  {
    switchListItem(list_used, list_available);
  }else{
    switchListItem(list_available, list_used);
  }
}

void MaterialManagement::material_card_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->selectListItem(current_item);
    this->loadWidget(current_item);
  }
}