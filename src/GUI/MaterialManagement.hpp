#ifndef MATERIALMANAGEMENT_HPP
#define MATERIALMANAGEMENT_HPP

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QString>
#include <QStringList>

class CalculiXCoreInterface;
class MaterialManagementItem;
class MaterialManagementElasticCard;
class MaterialManagementPlasticCard;
class MaterialManagementDensityCard;
class MaterialManagementSpecificHeatCard;
class MaterialManagementExpansionCard;
class MaterialManagementConductivityCard;
class MaterialManagementDeformationPlasticityCard;
class MaterialManagementMohrCoulombCard;
class MaterialManagementMohrCoulombHardeningCard;
class MaterialManagementCyclicHardeningCard;
class MaterialManagementRateDependentCard;
class MaterialManagementCreepCard;

class MaterialManagement : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagement();
  ~MaterialManagement();

  CalculiXCoreInterface *ccx_iface;
  
  void clear(); // remove all data
  void update();
  void update_cubit(); //update cubit materials
  void update_materiallibrary(); //update library materials

  void addMaterial(QString material_id, QString material_name); // adds a new material to the tree
  void removeMaterial(MaterialManagementItem *material); // removes the material from to the tree
  int  get_child_id(std::string material_id); // check if the item for the given material_id exists, returns the id or -1 if failed;
  bool check_library_item_exists(std::string path); // check if the item for the given path exists;
  MaterialManagementItem* get_library_item(std::string path); // gets the library item from the path, returns null if nothing is found
  void addMateriallibrary(std::vector<std::string> tree_data); // adds a new item to the tree
  void removeMateriallibrary(MaterialManagementItem *item); // removes the item from to the tree
  
  void createListItems(MaterialManagementItem *material); // creates the list items for selected material
  void removeListItems(); // removes current list items
  void switchListItem(QListWidget* source, QListWidget* target); // switches the current item from sry list to target list
  void selectListItem(QListWidgetItem* item); // unselect all list items except the given
  void loadWidget(QListWidgetItem* item); // load Widget for given list item
  void setMaterial(QString material_id);
  void printproperties(); // prints out material with all properties
  // cards

private slots:
  void on_pushButton_ok_clicked(bool);
  void on_pushButton_apply_clicked(bool);
  void on_pushButton_close_clicked(bool);
  void on_pushButton_new_clicked(bool);
  void on_pushButton_delete_clicked(bool);
  void on_pushButton_rename_clicked(bool);
  void on_pushButton_export_clicked(bool);
  void on_pushButton_add_clicked(bool);
  void on_pushButton_remove_clicked(bool);
  void on_pushButton_library_import_clicked(bool);
  void on_pushButton_library_new_clicked(bool);
  void on_pushButton_library_delete_clicked(bool);
  void on_pushButton_library_rename_clicked(bool);
  void on_pushButton_library_new_group_clicked(bool);
  void on_pushButton_library_delete_group_clicked(bool);
  void on_pushButton_library_rename_group_clicked(bool);
  void on_pushButton_library_description_clicked(bool);

  void material_clicked(QTreeWidgetItem* item, int column);
  void material_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item);
  void library_material_clicked(QTreeWidgetItem* item, int column);
  void library_material_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item);
  // cards
  void material_card_clicked(QListWidgetItem* item);
  void material_card_doubleclicked(QListWidgetItem* item);
  void material_card_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);
  
private:
  std::string log;
  MaterialManagementItem* material_item;
  MaterialManagementItem* current_material_item;
  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_material1;
  QVBoxLayout* boxLayout_material2;
  QVBoxLayout* boxLayout_material_cards1;
  QVBoxLayout* boxLayout_material_cards2;
  QVBoxLayout* boxLayout_material_cards3;
  QVBoxLayout* boxLayout_materiallibrary1;
  QVBoxLayout* boxLayout_materiallibrary2;
  QVBoxLayout* boxLayout_widget;
  QPushButton* pushButton_ok;
  QPushButton* pushButton_apply;
  QPushButton* pushButton_close;
  QPushButton* pushButton_new;
  QPushButton* pushButton_delete;
  QPushButton* pushButton_rename;
  QPushButton* pushButton_export;
  QPushButton* pushButton_add;
  QPushButton* pushButton_remove;
  QPushButton* pushButton_library_import;
  QPushButton* pushButton_library_new;
  QPushButton* pushButton_library_delete;
  QPushButton* pushButton_library_rename;
  QPushButton* pushButton_library_new_group;
  QPushButton* pushButton_library_delete_group;
  QPushButton* pushButton_library_rename_group;
  QPushButton* pushButton_library_description;
  QLabel* label_material;
  QLabel* label_available;
  QLabel* label_used;
  QLabel* label_materiallibrary;
  //QLabel* label_materiallibrary_material;
  QLabel* label_materiallibrary_group;
  QLabel* label_materiallibrary_description;
  QTreeWidget* tree_material;
  QListWidget* list_available;
  QListWidget* list_used;
  QTreeWidget* tree_materiallibrary;
  QListWidgetItem* list_elastic;
  QListWidgetItem* list_plastic;
  QListWidgetItem* list_density;
  QListWidgetItem* list_specific_heat;
  QListWidgetItem* list_expansion;
  QListWidgetItem* list_conductivity;
  QListWidgetItem* list_deformation_plasticity;
  QListWidgetItem* list_mohr_coulomb;
  QListWidgetItem* list_mohr_coulomb_hardening;
  QListWidgetItem* list_cylclic_hardening;
  QListWidgetItem* list_rate_dependent;
  QListWidgetItem* list_creep;
  QListWidgetItem* list_hyperelastic;
  QListWidgetItem* list_hyperfoam;
  QListWidgetItem* list_compression_only;
  QListWidgetItem* list_tension_only;
  QListWidgetItem* list_damping;

  // cards
  QFrame* card_frame;
  MaterialManagementElasticCard* elastic_widget;
  MaterialManagementPlasticCard* plastic_widget;
  MaterialManagementDensityCard* density_widget;
  MaterialManagementSpecificHeatCard* specific_heat_widget;
  MaterialManagementExpansionCard* expansion_widget;
  MaterialManagementConductivityCard* conductivity_widget;
  MaterialManagementDeformationPlasticityCard* deformation_plasticity_widget;
  MaterialManagementMohrCoulombCard* mohr_coulomb_widget;
  MaterialManagementMohrCoulombHardeningCard* mohr_coulomb_hardening_widget;
  MaterialManagementCyclicHardeningCard* cyclic_hardening_widget;
  MaterialManagementRateDependentCard* rate_dependent_widget;
  MaterialManagementCreepCard*  creep_widget;

};

#endif // MATERIALMANAGEMENT_HPP
