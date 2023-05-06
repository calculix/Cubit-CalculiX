#include "MaterialManagementTable.hpp"

MaterialManagementTable::MaterialManagementTable(QWidget* parent):
  QTableWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;

  //delegator = new QAbstractItemDelegate();
  this->setColumnCount(3);
  this->setRowCount(3);
  this->setItemDelegateForColumn(0,delegator);
  this->setHorizontalHeaderLabels({"Young's Modulus","Poisson's ratio","Temperature"});
  //this->setVerticalHeaderLabels({"1","2","3"});

  isInitialized = true;
}

MaterialManagementTable::~MaterialManagementTable()
{
}