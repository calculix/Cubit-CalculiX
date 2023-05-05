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
  int get_properties_data_id_from_group(std::string group); // searches for the group in the group_properties and returns the linked indices from the properties or -1 if it fails
  void setScalarPropertyGUI(std::string group, double prop_scalar); // sets the scalar property to given value
  double getScalarPropertyGUI(std::string group); // gets the scalar property value

  bool isInitialized;
  int material_id;
  QString material_id_qstring;
  std::string material_name;
  QString material_name_qstring;

  CalculiXCoreInterface *ccx_iface;
  MaterialInterface *mat_iface;
};

#endif // MATERIALMANAGEMENTITEM_HPP