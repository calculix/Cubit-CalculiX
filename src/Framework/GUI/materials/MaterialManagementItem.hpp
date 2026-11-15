#pragma once
#include "Framework/Core/materials/MaterialData.hpp"
#include <QObject>
#include <QTreeWidgetItem>
#include <QString>
#include <optional>
#include <map>
#include <string>
#include <vector>
class MaterialManagementItem : public QObject, public QTreeWidgetItem {
  Q_OBJECT
public:
  MaterialManagementItem(QTreeWidget* parent, std::string workspace = "calculix");
  MaterialManagementItem(QTreeWidgetItem* parent, std::string workspace = "calculix");
  ~MaterialManagementItem();
  void initialize_project(QString id, QString name);
  void initialize_library(const std::vector<std::string>& treeData);
  bool refresh();
  bool hasChanges() const;
  std::string validationMessage() const;
  bool hasPropertyError(const std::string& property) const;
  void setPropertyError(const std::string& property, const std::string& error);
  void clearPropertyError(const std::string& property);
  static bool equal(const MaterialData& first, const MaterialData& second);
  void setScalarPropertyGUI(const std::string& property, double value);
  double getScalarPropertyGUI(const std::string& property) const;
  void setMatrixPropertyGUI(const std::string& property, const std::vector<std::vector<double>>& values);
  std::vector<std::vector<double>> getMatrixPropertyGUI(const std::string& property) const;
  bool modelSelectedGUI(const std::string& card, const std::string& model) const;
  void selectModelGUI(const std::string& card, const std::string& model);
  bool cardActiveGUI(const std::string& card) const;
  void setCardActiveGUI(const std::string& card, bool active);
  std::optional<double> referenceTemperatureGUI() const;
  void setReferenceTemperatureGUI(double value);
  void clearReferenceTemperatureGUI();
  std::string workspace;
  std::string projectIdentity;
  MaterialData original;
  MaterialData draft;
  std::string validationError;
  std::map<std::string, std::string> propertyErrors;
  bool isInitialized = false;
  bool isProjectMaterial = false;
  bool isLibraryGroup = false;
  bool isLibraryMaterial = false;
  int material_id = 0;
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
};
