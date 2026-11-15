#pragma once
#include <QTableWidget>
#include <QString>
#include <string>
#include <cstdint>
#include <vector>
class MaterialManagementItem;
class MaterialManagementTable : public QTableWidget {
  Q_OBJECT
public:
  MaterialManagementTable(QWidget* parent, QString property);
  ~MaterialManagementTable();
  void update(MaterialManagementItem* material);
  std::vector<std::vector<double>> getMatrixPropertyGUI();
  void insertNewRow(std::vector<double> values);
  void insertLastRow();
  bool checkRowEmtpy(int row);
private slots:
  void cellChanged(int row, int column);
private:
  bool updating = false;
  std::uint64_t displayedGeneration = 0;
  MaterialManagementItem* material = nullptr;
  std::string property;
};
