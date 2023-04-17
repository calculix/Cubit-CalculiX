#ifndef MATERIALMANAGEMENTITEM_HPP
#define MATERIALMANAGEMENTITEM_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QString>

class CalculiXCoreInterface;

class MaterialManagementItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  MaterialManagementItem(QTreeWidget *parent);
  ~MaterialManagementItem();

  void initialize(QString material_id_qstring, QString material_name);

  void update(); // updates the data

private:
  bool isInitialized;
  int material_id;

  CalculiXCoreInterface *ccx_iface;
};

#endif // MATERIALMANAGEMENTITEM_HPP