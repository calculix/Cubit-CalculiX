#ifndef MATERIALMANAGEMENT_HPP
#define MATERIALMANAGEMENT_HPP

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;
class MaterialManagementItem;

class MaterialManagement : public QWidget
{
  Q_OBJECT
  
public:
  explicit MaterialManagement();
  ~MaterialManagement();

  CalculiXCoreInterface *ccx_iface;
  
  void clear(); // remove all data
  void update();

  void addMaterial(QString material_id, QString material_name); // adds a new material to the tree
  void removeMaterial(MaterialManagementItem *material); // removes the material from to the tree
  int get_child_id(std::string material_id); // check if the item for the given material_id exists, returns the id or -1 if failed;
  void createListItems(MaterialManagementItem *material); // creates the list items for selected material
  void removeListItems(); // removes current list items
  void switchListItem(QListWidget* source, QListWidget* target); // switches the current item from sry list to target list

  void printproperties(); // prints out material with all properties

private slots:
  void on_pushButton_ok_clicked(bool);
  void on_pushButton_apply_clicked(bool);
  void on_pushButton_close_clicked(bool);
  void on_pushButton_new_clicked(bool);
  void on_pushButton_delete_clicked(bool);
  void on_pushButton_add_clicked(bool);
  void on_pushButton_remove_clicked(bool);
  void material_clicked(QTreeWidgetItem* item, int column);

private:
  std::string log;
  QPushButton* pushButton_ok;
  QPushButton* pushButton_apply;
  QPushButton* pushButton_close;
  QPushButton* pushButton_new;
  QPushButton* pushButton_delete;
  QPushButton* pushButton_add;
  QPushButton* pushButton_remove;
  QLabel* label_material;
  QLabel* label_available;
  QLabel* label_used;
  QTreeWidget* tree_material;
  QListWidget* list_available;
  QListWidget* list_used;
  QListWidgetItem* list_elastic;
  QListWidgetItem* list_plastic;
  QListWidgetItem* list_density;
  QListWidgetItem* list_expansion;
  MaterialManagementItem* material_item;
};

#endif // MATERIALMANAGEMENT_HPP
