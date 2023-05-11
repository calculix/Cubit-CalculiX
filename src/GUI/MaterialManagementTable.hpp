#ifndef MATERIALMANAGEMENTTABLE_HPP
#define MATERIALMANAGEMENTTABLE_HPP

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAbstractItemDelegate>
#include <QHeaderView>

class MaterialManagementItem;

class MaterialManagementTable : public QTableWidget
{
  Q_OBJECT
  
public:
  MaterialManagementTable(QWidget *parent, QString group);
  ~MaterialManagementTable();
  void update(MaterialManagementItem *material); // updates the data in the table
  std::vector<std::vector<double>> getMatrixPropertyGUI(); // gets the matrix property values
  void insertNewRow(std::vector<double> values); // inserts new row at the bottom
  void insertLastRow(); // inserts new last row at the bottom
  bool checkRowEmtpy(int row); // checks if the columns in the row are empty

  bool isInitialized;
private slots:
  void currentItemChanged(QTableWidgetItem* current_item, QTableWidgetItem** prev_item);
  void currentCellChanged(int currentRow,int currentColumn,int previousRow,int previousColumn);
  void cellChanged(int row,int column);

private: 
bool bool_update=true;
std::string log;
QAbstractItemDelegate* delegator;
MaterialManagementItem* material;
std::string group;
int currentRow;
QStringList verticalHeaderLabels;
std::vector<std::vector<double>> property_matrix_gui;

};

#endif // MATERIALMANAGEMENTTABLE_HPP