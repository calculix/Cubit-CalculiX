#include "PanelTable.hpp"

#include "CubitMessage.hpp"

PanelTable::PanelTable(QWidget* parent):
  QTableWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  
  //this->setMinimumSize(200,200);
  this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  //this->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  // Signals
  QObject::connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),this,  SLOT(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)));
  QObject::connect(this, SIGNAL(currentCellChanged(int,int,int,int)),this,  SLOT(currentCellChanged(int,int,int,int)));
  QObject::connect(this, SIGNAL(cellChanged(int,int)),this,  SLOT(cellChanged(int,int)));

  isInitialized = true;
}

PanelTable::~PanelTable()
{
}

void PanelTable::update(QStringList columns,std::vector<std::vector<double>> matrix)
{
  bool_update=true;
  this->setRowCount(0);
  
  this->setColumnCount(columns.count());
  this->setHorizontalHeaderLabels(columns);
  //this->setColumnWidth(0,150);

  verticalHeaderLabels = QStringList();
  this->matrix = matrix;
  //property_matrix_gui = material->getMatrixPropertyGUI(group);
  //inser data from material
  for (size_t i = 0; i < matrix.size(); i++)
  {
    this->insertNewRow(matrix[i]);
  }
  
  // insert last row
  this->insertLastRow();
  
  bool_update = false;
}


std::vector<std::vector<double>> PanelTable::getMatrix()
{
  std::vector<std::vector<double>> matrix_return;
  std::vector<double> matrix_row;
  matrix_return.clear();
  //log = "ROW COUNT " + std::to_string(this->rowCount()) + "COLUMN COUNT " + std::to_string(this->columnCount()) + " \n";
  //PRINT_INFO("%s", log.c_str());

  for (size_t i = 0; i < this->rowCount()-1; i++)
  {
    matrix_row.clear();
    for (size_t ii = 0; ii < this->columnCount(); ii++)
    {
      QTableWidgetItem* item;
      item = this->item(int(i),int(ii));
      if (item!=NULL)
      {
        if (item->text()!="")
        {
          matrix_row.push_back(item->text().toDouble());
        }else{
          matrix_row.push_back(0.);
        }
      }else{
        matrix_row.push_back(0.);
      }
    }
    matrix_return.push_back(matrix_row);
  }

  return matrix_return;
}

void PanelTable::insertNewRow(std::vector<double> values)
{
  this->insertRow(this->rowCount());
  this->currentRow = this->rowCount()-1;
  verticalHeaderLabels.append(QString::number(this->rowCount()));

  for (size_t i = 0; i < values.size(); i++)
  {
    QTableWidgetItem* item;
    item = new QTableWidgetItem();
    item->setText(QString::number(values[i]));
    this->setItem(this->currentRow,int(i),item);
  }
}

void PanelTable::insertLastRow()
{
  this->insertRow(this->rowCount());
  //log = std::to_string(this->rowCount()) + " - row count\n";
  //PRINT_INFO("%s", log.c_str());

  for (size_t i = 0; i < this->columnCount(); i++)
  {
    QTableWidgetItem* item;
    item = new QTableWidgetItem();
    item->setText("");
    this->setItem(this->rowCount()-1,int(i),item);
  }

  if (verticalHeaderLabels.count()!=0)
  {
    verticalHeaderLabels[verticalHeaderLabels.count()-1]=QString::number(verticalHeaderLabels.count());
  }
  verticalHeaderLabels.append("*");
  this->setVerticalHeaderLabels(verticalHeaderLabels);
}

bool PanelTable::checkRowEmtpy(int row)
{
  QTableWidgetItem* item;
  bool bool_empty = true;

  for (size_t i = 0; i < this->columnCount()-1; i++)
  {
    item = this->item(row,int(i));
    if (item!=NULL)
    {
      if (item->text().toDouble()!=0.)
      {
        bool_empty = false;
      }
      if ((item->text().toDouble()==0.)&&(item->text()!="")&&(this->panel_type=="amplitude"))
      {
        bool_empty = false;
      }
    }
  }
  
  return bool_empty;
}

void PanelTable::currentItemChanged(QTableWidgetItem* current_item, QTableWidgetItem** prev_item)
{
  if (current_item!=nullptr)
  {
    
  }
}

void PanelTable::currentCellChanged(int currentRow,int currentColumn,int previousRow,int previousColumn)
{
  this->currentRow = currentRow;
}

void PanelTable::cellChanged(int row,int column)
{
  this->currentRow = row;

  // check if data was added to the last row
  if ((row==this->rowCount()-1) && (this->checkRowEmtpy(row)==false) && !bool_update)
  {
    this->insertLastRow();
    this->setCurrentCell(row, column);
  }

  //check if this row (except last row) is empty, when yes delete
  if (!(row==this->rowCount()-1) && (this->checkRowEmtpy(row)==true))
  {
    this->removeRow(row);
    
    verticalHeaderLabels.removeAt(row);
    for (size_t i = 0; i < verticalHeaderLabels.count()-1; i++)
    {
      verticalHeaderLabels[int(i)]=QString::number(i+1);
    }
    this->setVerticalHeaderLabels(verticalHeaderLabels);
  }
}