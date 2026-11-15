#include "MaterialManagement.hpp"
#include "MaterialManagementItem.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp"
#include "CubitMessage.hpp"
#include <QCloseEvent>
#include <QSignalBlocker>
#include <QTreeWidgetItemIterator>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QTimer>
#include <QStringList>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTreeWidget>
#include <QListWidget>
#include <QFrame>
#include "MaterialManagementElasticCard.hpp"
#include "MaterialManagementPlasticCard.hpp"
#include "MaterialManagementDensityCard.hpp"
#include "MaterialManagementExpansionCard.hpp"
#include "MaterialManagementSpecificHeatCard.hpp"
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

namespace {
std::string material_model_command(const std::string& model) {
  if (model == "iso") {
    return "isotropic";
  }
  if (model == "ortho") {
    return "orthotropic";
  }
  if (model == "ec") {
    return "engineering_constants";
  }
  if (model == "aniso") {
    return "anisotropic";
  }
  if (model == "kin") {
    return "kinematic";
  }
  return model;
}
}

MaterialManagement::MaterialManagement(std::string workspace, std::string prefix, std::function<bool(const std::string&, std::string&)> submit, QWidget* parent) : QWidget(parent, Qt::Window), workspaceId(std::move(workspace)), commandPrefix(std::move(prefix)), projectIdentity(framework_materials().project_id()), libraryFilename(framework_materials().library().filename()), submitCommand(std::move(submit)) {
  this->setGeometry(0,0,1520,800);
  this->setWindowTitle("Material Management - " + QString::fromStdString(workspaceId));
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

  pushButton_ok = new QPushButton();

  pushButton_ok->setText("OK");
  boxLayout_window->addWidget(pushButton_ok);

  pushButton_apply = new QPushButton();

  pushButton_apply->setText("Apply");
  boxLayout_window->addWidget(pushButton_apply);

  pushButton_close = new QPushButton();

  pushButton_close->setText("Close");
  boxLayout_window->addWidget(pushButton_close);

  pushButton_new = new QPushButton();

  pushButton_new->setText("New");
  boxLayout_material2->addWidget(pushButton_new);

  pushButton_delete = new QPushButton();

  pushButton_delete->setText("Delete");
  boxLayout_material2->addWidget(pushButton_delete);

  pushButton_rename = new QPushButton();
  pushButton_rename->setText("Rename");
  boxLayout_material2->addWidget(pushButton_rename);

  pushButton_export = new QPushButton();
  pushButton_export->setText("Export");
  boxLayout_material2->addWidget(pushButton_export);

  pushButton_add = new QPushButton();

  pushButton_add->setText("Activate >");
  boxLayout_material_cards2->addWidget(pushButton_add);

  pushButton_remove = new QPushButton();

  pushButton_remove->setText("< Deactivate");
  boxLayout_material_cards2->addWidget(pushButton_remove);




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

  label_material = new QLabel();

  label_material->setText("Material");
  boxLayout_material1->addWidget(label_material);

  label_available = new QLabel();

  label_available->setText("Inactive Cards");
  boxLayout_material_cards1->addWidget(label_available);

  label_used = new QLabel();

  label_used->setText("Active Cards");
  boxLayout_material_cards3->addWidget(label_used);

  label_materiallibrary = new QLabel();
  label_materiallibrary->setText("Library");
  boxLayout_materiallibrary1->addWidget(label_materiallibrary);

  tree_material = new QTreeWidget();

  tree_material->setColumnCount(2);
  tree_material->setColumnWidth(0, 200);
  tree_material->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_material1->addWidget(tree_material);

  list_available = new QListWidget();

  boxLayout_material_cards1->addWidget(list_available);

  list_used = new QListWidget();

  boxLayout_material_cards3->addWidget(list_used);

  tree_materiallibrary = new QTreeWidget();
  tree_materiallibrary->setColumnCount(2);
  tree_materiallibrary->setColumnWidth(0, 200);
  tree_materiallibrary->setHeaderLabels(QStringList() << "Name" << "Description");
  boxLayout_materiallibrary1->addWidget(tree_materiallibrary);

  card_frame = new QFrame();

  card_frame->setMinimumSize(700,350);
  card_frame->setLineWidth(1);
  card_frame->setMidLineWidth(0);
  card_frame->setFrameStyle(QFrame::Box | QFrame::Raised);


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


  cardWidgets.emplace("elastic", elastic_widget);
  cardUpdates.emplace("elastic", [this](MaterialManagementItem* item) {
    elastic_widget->update(item);
  });
  cardWidgets.emplace("plastic", plastic_widget);
  cardUpdates.emplace("plastic", [this](MaterialManagementItem* item) {
    plastic_widget->update(item);
  });
  cardWidgets.emplace("density", density_widget);
  cardUpdates.emplace("density", [this](MaterialManagementItem* item) {
    density_widget->update(item);
  });
  cardWidgets.emplace("expansion", expansion_widget);
  cardUpdates.emplace("expansion", [this](MaterialManagementItem* item) {
    expansion_widget->update(item);
  });
  cardWidgets.emplace("specific_heat", specific_heat_widget);
  cardUpdates.emplace("specific_heat", [this](MaterialManagementItem* item) {
    specific_heat_widget->update(item);
  });
  cardWidgets.emplace("conductivity", conductivity_widget);
  cardUpdates.emplace("conductivity", [this](MaterialManagementItem* item) {
    conductivity_widget->update(item);
  });
  cardWidgets.emplace("deformation_plasticity", deformation_plasticity_widget);
  cardUpdates.emplace("deformation_plasticity", [this](MaterialManagementItem* item) {
    deformation_plasticity_widget->update(item);
  });
  cardWidgets.emplace("mohr_coulomb", mohr_coulomb_widget);
  cardUpdates.emplace("mohr_coulomb", [this](MaterialManagementItem* item) {
    mohr_coulomb_widget->update(item);
  });
  cardWidgets.emplace("mohr_coulomb_hardening", mohr_coulomb_hardening_widget);
  cardUpdates.emplace("mohr_coulomb_hardening", [this](MaterialManagementItem* item) {
    mohr_coulomb_hardening_widget->update(item);
  });
  cardWidgets.emplace("cyclic_hardening", cyclic_hardening_widget);
  cardUpdates.emplace("cyclic_hardening", [this](MaterialManagementItem* item) {
    cyclic_hardening_widget->update(item);
  });
  cardWidgets.emplace("rate_dependent", rate_dependent_widget);
  cardUpdates.emplace("rate_dependent", [this](MaterialManagementItem* item) {
    rate_dependent_widget->update(item);
  });
  cardWidgets.emplace("creep", creep_widget);
  cardUpdates.emplace("creep", [this](MaterialManagementItem* item) {
    creep_widget->update(item);
  });
  cardWidgets.emplace("hyperelastic", hyperelastic_widget);
  cardUpdates.emplace("hyperelastic", [this](MaterialManagementItem* item) {
    hyperelastic_widget->update(item);
  });
  cardWidgets.emplace("hyperfoam", hyperfoam_widget);
  cardUpdates.emplace("hyperfoam", [this](MaterialManagementItem* item) {
    hyperfoam_widget->update(item);
  });
  cardWidgets.emplace("compression_only", compression_only_widget);
  cardUpdates.emplace("compression_only", [this](MaterialManagementItem* item) {
    compression_only_widget->update(item);
  });
  cardWidgets.emplace("tension_only", tension_only_widget);
  cardUpdates.emplace("tension_only", [this](MaterialManagementItem* item) {
    tension_only_widget->update(item);
  });
  cardWidgets.emplace("damping", damping_widget);
  cardUpdates.emplace("damping", [this](MaterialManagementItem* item) {
    damping_widget->update(item);
  });
  update();
}
MaterialManagement::~MaterialManagement() = default;
std::string MaterialManagement::quoted(const std::string& value) {
  std::string text = "\"";
  for (char character : value) {
    if (character == '\\' || character == '"') {
      text.push_back('\\');
    }
    text.push_back(character);
  }
  text.push_back('"');
  return text;
}
std::vector<MaterialManagementItem*> MaterialManagement::items() const {
  std::vector<MaterialManagementItem*> result;
  for (int index = 0; index < tree_material->topLevelItemCount(); ++index) {
    auto* item = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(index));
    if (item != nullptr) {
      result.push_back(item);
    }
  }
  QTreeWidgetItemIterator iterator(tree_materiallibrary);
  while (*iterator != nullptr) {
    auto* item = dynamic_cast<MaterialManagementItem*>(*iterator);
    if (item != nullptr && item->isLibraryMaterial) {
      result.push_back(item);
    }
    ++iterator;
  }
  return result;
}
bool MaterialManagement::hasChanges() const {
  for (const auto* item : items()) {
    if (item->hasChanges()) {
      return true;
    }
  }
  return false;
}
void MaterialManagement::hideCards() {
  for (const auto& entry : cardWidgets) {
    entry.second->hide();
  }
}
void MaterialManagement::removeListItems() {
  const QSignalBlocker inactiveBlocker(list_available);
  const QSignalBlocker activeBlocker(list_used);
  list_available->clear();
  list_used->clear();
  hideCards();
}
void MaterialManagement::clear() {
  const QSignalBlocker projectBlocker(tree_material);
  const QSignalBlocker libraryBlocker(tree_materiallibrary);
  current_material_item = nullptr;
  removeListItems();
  tree_material->clear();
  tree_materiallibrary->clear();
}
void MaterialManagement::update() {
  if (applying) {
    return;
  }
  if (projectIdentity != framework_materials().project_id()) {
    clear();
    projectIdentity = framework_materials().project_id();
  } else if (hasChanges()) {
    return;
  }
  int selectedId = 0;
  if (current_material_item != nullptr && current_material_item->isProjectMaterial) {
    selectedId = current_material_item->material_id;
  }
  const QSignalBlocker projectBlocker(tree_material);
  const QSignalBlocker libraryBlocker(tree_materiallibrary);
  clear();
  libraryFilename = framework_materials().library().filename();
  FrameworkMaterialsInterface materials(workspaceId);
  for (int id : materials.get_material_ids()) {
    auto* item = new MaterialManagementItem(tree_material, workspaceId);
    item->initialize_project(QString::number(id), QString::fromStdString(materials.get_material_name(id)));
  }
  std::map<std::string, MaterialManagementItem*> libraryGroups;
  const auto library = materials.get_materiallibrary_tree_data();
  for (const auto& record : library) {
    if (record.size() < 4 || record[3] != "Group") {
      continue;
    }
    MaterialManagementItem* item = nullptr;
    const std::string parentPath = record[2].substr(0, record[2].find_last_of('/'));
    auto parent = libraryGroups.find(parentPath);
    if (parent == libraryGroups.end()) {
      item = new MaterialManagementItem(tree_materiallibrary, workspaceId);
    } else {
      item = new MaterialManagementItem(parent->second, workspaceId);
    }
    item->initialize_library(record);
    libraryGroups.emplace(record[2], item);
  }
  for (const auto& record : library) {
    if (record.size() < 4 || record[3] != "Material") {
      continue;
    }
    const std::string parentPath = record[2].substr(0, record[2].find_last_of('/'));
    auto parent = libraryGroups.find(parentPath);
    if (parent == libraryGroups.end()) {
      PRINT_ERROR("Material library group is missing from its tree.\n");
      continue;
    }
    auto* item = new MaterialManagementItem(parent->second, workspaceId);
    item->initialize_library(record);
  }
  tree_materiallibrary->expandToDepth(0);
  if (selectedId > 0) {
    setMaterial(QString::number(selectedId));
  }
}
void MaterialManagement::setMaterial(QString id) {
  for (int index = 0; index < tree_material->topLevelItemCount(); ++index) {
    auto* item = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(index));
    if (item != nullptr && item->material_id_qstring == id) {
      tree_material->setCurrentItem(item);
      tree_material->scrollToItem(item);
      material_clicked(item, 0);
      return;
    }
  }
}
void MaterialManagement::createListItems(MaterialManagementItem* material) {
  removeListItems();
  if (material == nullptr || (!material->isProjectMaterial && !material->isLibraryMaterial)) {
    return;
  }
  for (const auto& definition : framework_materials().definitions().cards()) {
    auto* item = new QListWidgetItem(QString::fromStdString(definition.title));
    item->setData(Qt::UserRole, QString::fromStdString(definition.key));
    if (material->cardActiveGUI(definition.key)) {
      list_used->addItem(item);
    } else {
      list_available->addItem(item);
    }
  }
}
void MaterialManagement::loadWidget(QListWidgetItem* item) {
  hideCards();
  if (item == nullptr || current_material_item == nullptr) {
    return;
  }
  const std::string key = item->data(Qt::UserRole).toString().toStdString();
  auto widget = cardWidgets.find(key);
  auto refresh = cardUpdates.find(key);
  if (widget != cardWidgets.end() && refresh != cardUpdates.end()) {
    refresh->second(current_material_item);
    widget->second->show();
  }
}
void MaterialManagement::material_clicked(QTreeWidgetItem* item, int) {
  if (!canSelectMaterial(item)) {
    return;
  }
  const QSignalBlocker blocker(tree_materiallibrary);
  tree_materiallibrary->clearSelection();
  current_material_item = dynamic_cast<MaterialManagementItem*>(item);
  createListItems(current_material_item);
}
void MaterialManagement::material_changed(QTreeWidgetItem* current, QTreeWidgetItem*) {
  if (current != nullptr) {
    material_clicked(current, 0);
  }
}
void MaterialManagement::library_material_clicked(QTreeWidgetItem* item, int) {
  if (!canSelectMaterial(item)) {
    return;
  }
  const QSignalBlocker blocker(tree_material);
  tree_material->clearSelection();
  current_material_item = dynamic_cast<MaterialManagementItem*>(item);
  createListItems(current_material_item);
}
void MaterialManagement::library_material_changed(QTreeWidgetItem* current, QTreeWidgetItem*) {
  if (current != nullptr) {
    library_material_clicked(current, 0);
  }
}
bool MaterialManagement::canSelectMaterial(QTreeWidgetItem* item) {
  if (current_material_item == nullptr || current_material_item == item || current_material_item->validationMessage().empty()) {
    return true;
  }
  PRINT_ERROR("Correct the invalid material input or close and discard it before selecting another material.\n");
  const QSignalBlocker projectBlocker(tree_material);
  const QSignalBlocker libraryBlocker(tree_materiallibrary);
  tree_material->setCurrentItem(nullptr);
  tree_materiallibrary->setCurrentItem(nullptr);
  if (current_material_item->isProjectMaterial) {
    tree_material->setCurrentItem(current_material_item);
  } else {
    tree_materiallibrary->setCurrentItem(current_material_item);
  }
  return false;
}
void MaterialManagement::material_card_clicked(QListWidgetItem* item) {
  loadWidget(item);
}
void MaterialManagement::material_card_changed(QListWidgetItem* current, QListWidgetItem*) {
  if (current != nullptr) {
    loadWidget(current);
  }
}
void MaterialManagement::material_card_doubleclicked(QListWidgetItem* item) {
  if (item == nullptr || current_material_item == nullptr) {
    return;
  }
  const std::string key = item->data(Qt::UserRole).toString().toStdString();
  current_material_item->setCardActiveGUI(key, !current_material_item->cardActiveGUI(key));
  createListItems(current_material_item);
}
std::string MaterialManagement::target(const MaterialManagementItem& item) const {
  if (item.isProjectMaterial) {
    return commandPrefix + " modify material " + std::to_string(item.material_id);
  }
  return commandPrefix + " modify materiallibrary material name " + quoted(item.library_name) + " groupname " + quoted(item.library_group);
}
bool MaterialManagement::applyChanges() {
  if (projectIdentity != framework_materials().project_id()) {
    PRINT_ERROR("The project changed. Reopen Material Management before applying edits.\n");
    return false;
  }
  const auto materialItems = items();
  for (auto* item : materialItems) {
    if (!item->hasChanges()) {
      continue;
    }
    if (item->isLibraryMaterial && libraryFilename != framework_materials().library().filename()) {
      PRINT_ERROR("The material library file changed. Close and discard this library draft before reopening it.\n");
      return false;
    }
    const std::string validation = item->validationMessage();
    if (!validation.empty()) {
      PRINT_ERROR("%s\n", validation.c_str());
      return false;
    }
    MaterialData live;
    std::string error;
    if (item->isProjectMaterial) {
      FrameworkMaterialsInterface materials(workspaceId);
      const auto* current = materials.find(item->material_id);
      if (current == nullptr) {
        PRINT_ERROR("The edited material no longer exists.\n");
        return false;
      }
      live = current->data;
    } else if (!framework_materials().library().read_material(workspaceId, item->library_name, item->library_group, live, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      return false;
    }
    if (!MaterialManagementItem::equal(live, item->original)) {
      PRINT_ERROR("Material %s changed outside this editor. Close and discard this draft before reopening it.\n", live.name.c_str());
      return false;
    }
    if (!framework_materials().definitions().validate(item->draft, true, error)) {
      PRINT_ERROR("%s: %s\n", item->draft.name.c_str(), error.c_str());
      return false;
    }
  }
  applying = true;
  for (auto* item : materialItems) {
    if (!item->hasChanges()) {
      continue;
    }
    const std::string prefix = target(*item);
    std::vector<std::string> commands;
    for (const auto& previous : item->original.cards) {
      if (item->draft.cards.count(previous.first) == 0) {
        commands.push_back(prefix + " remove card " + previous.first);
      }
    }
    for (const auto& entry : item->draft.cards) {
      const MaterialCard& card = entry.second;
      const std::string modelName = material_model_command(card.model);
      auto previous = item->original.cards.find(entry.first);
      if (previous != item->original.cards.end()) {
        MaterialData before;
        MaterialData after;
        before.cards.emplace(previous->first, previous->second);
        after.cards.emplace(entry.first, entry.second);
        if (MaterialManagementItem::equal(before, after)) {
          continue;
        }
      } else {
        commands.push_back(prefix + " add card " + entry.first + " model " + modelName);
      }
      commands.push_back(prefix + " deactivate card " + entry.first);
      commands.push_back(prefix + " card " + entry.first + " model " + modelName);
      commands.push_back(prefix + " card " + entry.first + " clear values");
      if (card.values.scalar.has_value()) {
        commands.push_back(prefix + " card " + entry.first + " value " + QString::number(*card.values.scalar, 'g', 17).toStdString() + " unit \"SI\"");
      }
      for (std::size_t row = 0; row < card.values.rows.size(); ++row) {
        std::string command = prefix + " card " + entry.first + " row " + std::to_string(row + 1) + " values";
        for (double value : card.values.rows[row].values) {
          command += " " + QString::number(value, 'g', 17).toStdString();
        }
        command += " unit \"SI\"";
        commands.push_back(std::move(command));
      }
      if (entry.first == "expansion") {
        if (card.values.referenceTemperature.has_value()) {
          commands.push_back(prefix + " card expansion reference temperature " + QString::number(*card.values.referenceTemperature, 'g', 17).toStdString() + " unit \"SI\"");
        } else {
          commands.push_back(prefix + " card expansion clear reference temperature");
        }
      }
      if (card.state == MaterialCardState::Active) {
        commands.push_back(prefix + " activate card " + entry.first);
      }
    }
    if (item->original.description != item->draft.description) {
      commands.push_back(prefix + " description " + quoted(item->draft.description));
    }
    if (item->original.name != item->draft.name) {
      commands.push_back(prefix + " name " + quoted(item->draft.name));
    }
    for (const std::string& command : commands) {
      std::string error;
      if (!submitCommand || !submitCommand(command, error)) {
        PRINT_ERROR("Cannot submit material edit: %s\n", error.c_str());
        applying = false;
        return false;
      }
    }
    MaterialData live;
    std::string error;
    if (item->isProjectMaterial) {
      FrameworkMaterialsInterface materials(workspaceId);
      const auto* current = materials.find(item->material_id);
      if (current == nullptr) {
        PRINT_ERROR("Material disappeared while applying edits.\n");
        applying = false;
        return false;
      }
      live = current->data;
    } else if (!framework_materials().library().read_material(workspaceId, item->draft.name, item->library_group, live, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      applying = false;
      return false;
    }
    if (!MaterialManagementItem::equal(live, item->draft)) {
      item->original = std::move(live);
      PRINT_ERROR("Material edits were not fully applied. The draft is retained; check the command errors and apply again.\n");
      applying = false;
      return false;
    }
    item->original = live;
    item->draft = std::move(live);
    item->validationError.clear();
    item->propertyErrors.clear();
    if (item->isLibraryMaterial) {
      item->library_name = item->draft.name;
    }
  }
  applying = false;
  update();
  return true;
}
bool MaterialManagement::sendAction(const std::string& command) {
  if (command.find("materiallibrary") != std::string::npos && libraryFilename != framework_materials().library().filename()) {
    PRINT_ERROR("The material library file changed. Refresh Material Management before using its library selection.\n");
    update();
    return false;
  }
  if (hasChanges() && !applyChanges()) {
    return false;
  }
  std::string error;
  if (!submitCommand || !submitCommand(command, error)) {
    PRINT_ERROR("Cannot submit material command: %s\n", error.c_str());
    return false;
  }
  update();
  return true;
}
void MaterialManagement::on_pushButton_ok_clicked(bool) {
  if (applyChanges()) {
    requestClose();
  }
}
void MaterialManagement::on_pushButton_apply_clicked(bool) {
  applyChanges();
}
void MaterialManagement::on_pushButton_close_clicked(bool) {
  requestClose();
}
void MaterialManagement::requestClose() {
  std::string error;
  if (!submitCommand || !submitCommand(commandPrefix + " close materialmanagement", error)) {
    PRINT_ERROR("Cannot close Material Management: %s\n", error.c_str());
  }
}
bool MaterialManagement::close_from_command() {
  closingFromCommand = true;
  const bool closed = QWidget::close();
  closingFromCommand = false;
  return closed;
}
void MaterialManagement::closeEvent(QCloseEvent* event) {
  if (!closingFromCommand) {
    event->ignore();
    QTimer::singleShot(0, this, [this]() {
      requestClose();
    });
    return;
  }
  if (applying) {
    event->ignore();
    return;
  }
  if (hasChanges()) {
    const auto answer = QMessageBox::question(this, "Material changes", "Apply the material changes before closing?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel || (answer == QMessageBox::Save && !applyChanges())) {
      event->ignore();
      return;
    }
    if (answer == QMessageBox::Discard) {
      clear();
    }
  }
  event->accept();
}
void MaterialManagement::on_pushButton_new_clicked(bool) {
  bool accepted = false;
  const QString name = QInputDialog::getText(this, "Create Material", "Material Name:", QLineEdit::Normal, "", &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(commandPrefix + " create material " + quoted(name.toStdString()));
  }
}
void MaterialManagement::on_pushButton_delete_clicked(bool) {
  if (current_material_item == nullptr || !current_material_item->isProjectMaterial) {
    return;
  }
  const int id = current_material_item->material_id;
  const auto answer = QMessageBox::question(this, "Delete Material", "Delete " + current_material_item->material_name_qstring + "?", QMessageBox::Yes | QMessageBox::No);
  if (answer == QMessageBox::Yes) {
    sendAction(commandPrefix + " delete material " + std::to_string(id));
  }
}
void MaterialManagement::on_pushButton_rename_clicked(bool) {
  if (current_material_item == nullptr || !current_material_item->isProjectMaterial) {
    return;
  }
  const int id = current_material_item->material_id;
  bool accepted = false;
  const QString name = QInputDialog::getText(this, "Rename Material", "Material Name:", QLineEdit::Normal, current_material_item->material_name_qstring, &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(commandPrefix + " modify material " + std::to_string(id) + " name " + quoted(name.toStdString()));
  }
}
MaterialManagementItem* MaterialManagement::selectedLibraryItem() const {
  return dynamic_cast<MaterialManagementItem*>(tree_materiallibrary->currentItem());
}
std::string MaterialManagement::selectedLibraryGroup() const {
  const auto* item = selectedLibraryItem();
  if (item == nullptr) {
    return {};
  }
  return item->library_group;
}
void MaterialManagement::on_pushButton_export_clicked(bool) {
  if (current_material_item == nullptr || !current_material_item->isProjectMaterial) {
    return;
  }
  const std::string materialName = current_material_item->material_name;
  std::string group = selectedLibraryGroup();
  bool accepted = false;
  if (group.empty()) {
    const QString value = QInputDialog::getText(this, "Export Material", "Library Group:", QLineEdit::Normal, "", &accepted);
    if (!accepted || value.trimmed().isEmpty()) {
      return;
    }
    group = value.toStdString();
  }
  const QString name = QInputDialog::getText(this, "Export Material", "Library Material Name:", QLineEdit::Normal, QString::fromStdString(materialName), &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(commandPrefix + " export materiallibrary material_name " + quoted(materialName) + " name " + quoted(name.toStdString()) + " groupname " + quoted(group));
  }
}
void MaterialManagement::on_pushButton_add_clicked(bool) {
  auto* item = list_available->currentItem();
  if (item == nullptr || current_material_item == nullptr) {
    return;
  }
  current_material_item->setCardActiveGUI(item->data(Qt::UserRole).toString().toStdString(), true);
  createListItems(current_material_item);
}
void MaterialManagement::on_pushButton_remove_clicked(bool) {
  auto* item = list_used->currentItem();
  if (item == nullptr || current_material_item == nullptr) {
    return;
  }
  current_material_item->setCardActiveGUI(item->data(Qt::UserRole).toString().toStdString(), false);
  createListItems(current_material_item);
}
void MaterialManagement::on_pushButton_library_import_clicked(bool) {
  const auto* item = selectedLibraryItem();
  if (item == nullptr || !item->isLibraryMaterial) {
    return;
  }
  const std::string libraryName = item->library_name;
  const std::string group = item->library_group;
  bool accepted = false;
  const QString name = QInputDialog::getText(this, "Import Material", "Project Material Name:", QLineEdit::Normal, item->library_name_qstring, &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction("sim import materiallibrary name " + quoted(libraryName) + " groupname " + quoted(group) + " workspace " + workspaceId + " material_name " + quoted(name.toStdString()));
  }
}
void MaterialManagement::on_pushButton_library_new_clicked(bool) {
  const std::string group = selectedLibraryGroup();
  if (group.empty()) {
    PRINT_ERROR("Select a material library group first.\n");
    return;
  }
  bool accepted = false;
  const QString name = QInputDialog::getText(this, "Create Library Material", "Material Name:", QLineEdit::Normal, "", &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(commandPrefix + " create materiallibrary material name " + quoted(name.toStdString()) + " groupname " + quoted(group));
  }
}
void MaterialManagement::on_pushButton_library_delete_clicked(bool) {
  const auto* item = selectedLibraryItem();
  if (item == nullptr || !item->isLibraryMaterial) {
    return;
  }
  const std::string name = item->library_name;
  const std::string group = item->library_group;
  const auto answer = QMessageBox::question(this, "Delete Library Material", "Delete " + item->library_name_qstring + "?", QMessageBox::Yes | QMessageBox::No);
  if (answer == QMessageBox::Yes) {
    sendAction(commandPrefix + " delete materiallibrary material name " + quoted(name) + " groupname " + quoted(group));
  }
}
void MaterialManagement::on_pushButton_library_rename_clicked(bool) {
  const auto* item = selectedLibraryItem();
  if (item == nullptr || !item->isLibraryMaterial) {
    return;
  }
  const std::string prefix = target(*item);
  bool accepted = false;
  const QString name = QInputDialog::getText(this, "Rename Library Material", "Material Name:", QLineEdit::Normal, item->library_name_qstring, &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(prefix + " name " + quoted(name.toStdString()));
  }
}
void MaterialManagement::on_pushButton_library_new_group_clicked(bool) {
  bool accepted = false;
  QString initial;
  const std::string parent = selectedLibraryGroup();
  if (!parent.empty()) {
    initial = QString::fromStdString(parent) + "/";
  }
  const QString name = QInputDialog::getText(this, "Create Library Group", "Group Name:", QLineEdit::Normal, initial, &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(commandPrefix + " create materiallibrary group name " + quoted(name.toStdString()));
  }
}
void MaterialManagement::on_pushButton_library_delete_group_clicked(bool) {
  const auto* item = selectedLibraryItem();
  if (item == nullptr || !item->isLibraryGroup) {
    return;
  }
  const std::string group = item->library_group;
  const auto answer = QMessageBox::question(this, "Delete Library Group", "Delete " + item->library_group_qstring + "?", QMessageBox::Yes | QMessageBox::No);
  if (answer == QMessageBox::Yes) {
    sendAction(commandPrefix + " delete materiallibrary group name " + quoted(group));
  }
}
void MaterialManagement::on_pushButton_library_rename_group_clicked(bool) {
  const auto* item = selectedLibraryItem();
  if (item == nullptr || !item->isLibraryGroup) {
    return;
  }
  const std::string group = item->library_group;
  bool accepted = false;
  const QString name = QInputDialog::getText(this, "Rename Library Group", "Group Name:", QLineEdit::Normal, item->library_group_qstring, &accepted);
  if (accepted && !name.trimmed().isEmpty()) {
    sendAction(commandPrefix + " modify materiallibrary group name " + quoted(group) + " new_name " + quoted(name.toStdString()));
  }
}
void MaterialManagement::on_pushButton_library_description_clicked(bool) {
  const auto* item = selectedLibraryItem();
  if (item == nullptr) {
    return;
  }
  std::string prefix;
  if (item->isLibraryGroup) {
    prefix = commandPrefix + " modify materiallibrary group name " + quoted(item->library_group);
  } else {
    prefix = target(*item);
  }
  bool accepted = false;
  const QString description = QInputDialog::getText(this, "Library Description", "Description:", QLineEdit::Normal, item->library_description_qstring, &accepted);
  if (accepted) {
    sendAction(prefix + " description " + quoted(description.toStdString()));
  }
}
