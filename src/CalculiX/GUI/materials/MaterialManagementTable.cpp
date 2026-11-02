#include "MaterialCardPresentation.hpp"
#include "MaterialManagementTable.hpp"
#include "MaterialManagementItem.hpp"

#include "CubitMessage.hpp"

MaterialManagementTable::MaterialManagementTable(QWidget* parent, QString group_name):
  QTableWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;

  this->group = group_name.toStdString();
  
  //delegator = new QAbstractItemDelegate();
  //this->setItemDelegateForColumn(0,delegator);
  this->setMinimumSize(1100,240);
  //this->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  
  // Signals
  QObject::connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)),this,  SLOT(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)));
  QObject::connect(this, SIGNAL(currentCellChanged(int,int,int,int)),this,  SLOT(currentCellChanged(int,int,int,int)));
  QObject::connect(this, SIGNAL(cellChanged(int,int)),this,  SLOT(cellChanged(int,int)));

  isInitialized = true;
}

MaterialManagementTable::~MaterialManagementTable()
{
}

void MaterialManagementTable::update(MaterialManagementItem *material)
{
  bool_update=true;
  this->setRowCount(0);
  
  if (this->group=="CCX_ELASTIC_ISO_MODULUS_VS_POISSON_VS_TEMPERATURE")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
  }else if (this->group=="CCX_ELASTIC_ORTHO_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(10);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_ELASTIC_EC_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(10);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_ELASTIC_ANISO_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(22);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_PLASTIC_ISO_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
    this->setColumnWidth(1,150);
  }else if (this->group=="CCX_PLASTIC_KIN_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
    this->setColumnWidth(1,150);
  }else if (this->group=="CCX_PLASTIC_COMBINED_YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
    this->setColumnWidth(1,150);
  }else if (this->group=="CCX_PLASTIC_JOHNSON_COOK")
  {
    this->setColumnCount(6);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
    this->setColumnWidth(1,150);
  }else if (this->group=="CCX_DENSITY_DENSITY")
  {
    this->setColumnCount(2);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_SPECIFIC_HEAT_SPECIFIC_HEAT")
  {
    this->setColumnCount(2);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_EXPANSION_ISO_A_TEMPERATURE")
  {
    this->setColumnCount(2);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_EXPANSION_ORTHO_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_EXPANSION_ANISO_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(7);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_CONDUCTIVITY_ISO_K_TEMPERATURE")
  {
    this->setColumnCount(2);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_CONDUCTIVITY_ORTHO_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_CONDUCTIVITY_ANISO_CONSTANTS_VS_TEMPERATURE")
  {
    this->setColumnCount(7);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_DEFORMATION_PLASTICITY_RAMBERG_OSGOOD")
  {
    this->setColumnCount(6);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
    this->setColumnWidth(1,150);
    this->setColumnWidth(2,150);
    this->setColumnWidth(3,150);
    this->setColumnWidth(4,150);
    this->setColumnWidth(5,150);
  }else if (this->group=="CCX_MOHR_COULOMB_MOHR_COULOMB")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,300);
    this->setColumnWidth(1,300);
    this->setColumnWidth(2,150);
  }else if (this->group=="CCX_MOHR_COULOMB_HARDENING_MOHR_COULOMB_HARDENING")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,300);
    this->setColumnWidth(1,300);
    this->setColumnWidth(2,150);
  }else if (this->group=="CCX_CYCLIC_HARDENING_CYCLIC_HARDENING")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,300);
    this->setColumnWidth(1,300);
    this->setColumnWidth(2,150);
  }else if (this->group=="CCX_RATE_DEPENDENT_RATE_DEPENDENT")
  {
    this->setColumnCount(2);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
  }else if (this->group=="CCX_CREEP_CREEP")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(3,150);
  }else if (this->group=="CCX_HYPERELASTIC_ARRUDA_BOYCE")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(3,150);
  }else if (this->group=="CCX_HYPERELASTIC_MOONEY_RIVLIN")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(3,150);
  }else if (this->group=="CCX_HYPERELASTIC_NEO_HOOKE")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(2,150);
  }else if (this->group=="CCX_HYPERELASTIC_OGDEN_N1")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(3,150);
  }else if (this->group=="CCX_HYPERELASTIC_OGDEN_N2")
  {
    this->setColumnCount(7);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(6,150);
  }else if (this->group=="CCX_HYPERELASTIC_OGDEN_N3")
  {
    this->setColumnCount(10);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(9,150);
  }else if (this->group=="CCX_HYPERELASTIC_POLYNOMIAL_N1")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(3,150);
  }else if (this->group=="CCX_HYPERELASTIC_POLYNOMIAL_N2")
  {
    this->setColumnCount(8);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(7,150);
  }else if (this->group=="CCX_HYPERELASTIC_POLYNOMIAL_N3")
  {
    this->setColumnCount(13);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(12,150);
  }else if (this->group=="CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N1")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(2,150);
  }else if (this->group=="CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N2")
  {
    this->setColumnCount(5);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(4,150);
  }else if (this->group=="CCX_HYPERELASTIC_REDUCED_POLYNOMIAL_N3")
  {
    this->setColumnCount(7);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(6,150);
  }else if (this->group=="CCX_HYPERELASTIC_YEOH")
  {
    this->setColumnCount(7);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(6,150);
  }else if (this->group=="CCX_HYPERFOAM_HYPERFOAM_N1")
  {
    this->setColumnCount(4);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(3,150);
  }else if (this->group=="CCX_HYPERFOAM_HYPERFOAM_N2")
  {
    this->setColumnCount(7);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(6,150);
  }else if (this->group=="CCX_HYPERFOAM_HYPERFOAM_N3")
  {
    this->setColumnCount(10);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(9,150);
  }else if (this->group=="CCX_COMPRESSION_ONLY_COMPRESSION_ONLY")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(1,300);
    this->setColumnWidth(2,150);
  }else if (this->group=="CCX_TENSION_ONLY_TENSION_ONLY")
  {
    this->setColumnCount(3);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(1,300);
    this->setColumnWidth(2,150);
  }/*else if (this->group=="CCX_DAMPING_DAMPING")
  {
    this->setColumnCount(1);
    this->setHorizontalHeaderLabels(MaterialCardPresentation::columns(this->group));
    this->setColumnWidth(0,150);
  }*/

  verticalHeaderLabels = QStringList();
  this->material = material;
  property_matrix_gui = material->getMatrixPropertyGUI(group);
  //inser data from material
  for (size_t i = 0; i < property_matrix_gui.size(); i++)
  {
    this->insertNewRow(property_matrix_gui[i]);
  }
  
  // insert last row
  this->insertLastRow();
  bool_update = false;
}


std::vector<std::vector<double>> MaterialManagementTable::getMatrixPropertyGUI()
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

void MaterialManagementTable::insertNewRow(std::vector<double> values)
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

void MaterialManagementTable::insertLastRow()
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

bool MaterialManagementTable::checkRowEmtpy(int row)
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
    }
  }
  
  return bool_empty;
}

void MaterialManagementTable::currentItemChanged(QTableWidgetItem* current_item, QTableWidgetItem** prev_item)
{
  if (current_item!=nullptr)
  {
    
  }
}

void MaterialManagementTable::currentCellChanged(int currentRow,int currentColumn,int previousRow,int previousColumn)
{
  this->currentRow = currentRow;
}

void MaterialManagementTable::cellChanged(int row,int column)
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

  material->setMatrixPropertyGUI(group,this->getMatrixPropertyGUI());
  /*
  log = "matrix gui \n";
  std::vector<std::vector<double>> matrix = this->getMatrixPropertyGUI();
  for (size_t i = 0; i < matrix.size(); i++)
  {
    for (size_t ii = 0; ii < matrix[i].size(); ii++)
    {
      log.append(std::to_string(matrix[i][ii]) + " ");
    }
    log.append(" \n");
  }
  PRINT_INFO("%s", log.c_str()); 
  */
}