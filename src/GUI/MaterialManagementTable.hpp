#ifndef MATERIALMANAGEMENTTABLE_HPP
#define MATERIALMANAGEMENTTABLE_HPP

#include <QWidget>
#include <QTableWidget>
#include <QAbstractItemDelegate>

class MaterialManagementTable : public QTableWidget
{
  Q_OBJECT
  
public:
  MaterialManagementTable(QWidget *parent);
  ~MaterialManagementTable();
  bool isInitialized;
  
/*
private slots:
  void on_pushButton_ok_clicked(bool);
*/
  
//private: 
QAbstractItemDelegate* delegator;
};

#endif // MATERIALMANAGEMENTTABLE_HPP