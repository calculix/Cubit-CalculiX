#ifndef MATERIALMANAGEMENT_HPP
#define MATERIALMANAGEMENT_HPP

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
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

  void printproperties(); // prints out material with all properties

private slots:
  void on_pushButton_ok_clicked(bool);
  void on_pushButton_apply_clicked(bool);
  void on_pushButton_close_clicked(bool);
  void on_pushButton_new_clicked(bool);
  void on_pushButton_delete_clicked(bool);
  void on_pushButton_add_clicked(bool);
  void on_pushButton_remove_clicked(bool);

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
  MaterialManagementItem* material_item;
};

#endif // MATERIALMANAGEMENT_HPP
