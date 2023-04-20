#ifndef MATERIALMANAGEMENTITEM_HPP
#define MATERIALMANAGEMENTITEM_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QString>

class CalculiXCoreInterface;
class MaterialInterface;

class MaterialManagementItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  MaterialManagementItem(QTreeWidget *parent);
  ~MaterialManagementItem();

  std::vector<std::vector<std::string>> group_properties;
  // material group properties from core

  std::vector<std::vector<int>> properties;
  // properties[i][0] group id
  // properties[i][1] property type 1 scalar, 2 vector, 3 tabular, 4 matrix
  // properties[i][2] property id

  std::vector<double> property_scalar;
  std::vector<double> property_scalar_gui;
  // property_scalar[i] scalar
  
  std::vector<std::vector<double>> property_vector;
  std::vector<std::vector<double>> property_vector_gui;
  // property_scalar[i][ii] vector

  std::vector<std::vector<std::vector<double>>> property_tabular;
  std::vector<std::vector<std::vector<double>>> property_tabular_gui;
  // property_scalar[i][ii][iii] tabular

  std::vector<std::vector<std::vector<double>>> property_matrix;
  std::vector<std::vector<std::vector<double>>> property_matrix_gui;
  // property_scalar[i][ii][iii] matrix

  void initialize(QString material_id_qstring, QString material_name_qstring);
  void update(); // updates the data

  bool isInitialized;
  int material_id;
  std::string material_name;

  CalculiXCoreInterface *ccx_iface;
  MaterialInterface *mat_iface;
};

#endif // MATERIALMANAGEMENTITEM_HPP