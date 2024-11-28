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
  MaterialManagementItem(QTreeWidgetItem *parent);
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

  void initialize_cubit(QString material_id_qstring, QString material_name_qstring);
  void initialize_library(std::vector<std::string> tree_data);
  void update(); // updates the data
  void update_cubit(); // updates the data
  void update_library(); // updates the data
  int get_properties_data_id_from_group(std::string group); // searches for the group in the group_properties and returns the linked indices from the properties or -1 if it fails
  void setScalarPropertyGUI(std::string group, double prop_scalar); // sets the scalar property to given value
  double getScalarPropertyGUI(std::string group); // gets the scalar property value
  void setMatrixPropertyGUI(std::string group, std::vector<std::vector<double>> matrix); // sets the matrix property values to gui storage
  std::vector<std::vector<double>> getMatrixPropertyGUI(std::string group); // gets the matrix property values

  bool isInitialized;
  bool isCubit;
  bool isLibraryGroup;
  bool isLibraryMaterial;
  int material_id;
  QString material_id_qstring;
  std::string material_name;
  QString material_name_qstring;


  QString library_name_qstring;
  std::string library_name;
  QString library_description_qstring;
  std::string library_description;
  QString library_group_qstring;
  std::string library_group;

  std::string hdf5path;

  CalculiXCoreInterface *ccx_iface;
  MaterialInterface *mat_iface;
};

#endif // MATERIALMANAGEMENTITEM_HPP