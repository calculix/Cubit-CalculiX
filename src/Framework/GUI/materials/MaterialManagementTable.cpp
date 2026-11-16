#include "MaterialManagementTable.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialCardPresentation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitMessage.hpp"
#include <QHeaderView>
#include <QSignalBlocker>
#include <cmath>
#include <utility>
MaterialManagementTable::MaterialManagementTable(QWidget* parent, QString name) : QTableWidget(parent), property(name.toStdString()) {
  setMinimumSize(700, 180);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(this, &QTableWidget::cellChanged, this, &MaterialManagementTable::cellChanged);
}
MaterialManagementTable::~MaterialManagementTable() = default;
void MaterialManagementTable::update(MaterialManagementItem* selected) {
  if (selected != nullptr && selected == material && selected->hasPropertyError(property) && displayedGeneration == framework_units().generation()) {
    return;
  }
  const QSignalBlocker blocker(this);
  updating = true;
  material = selected;
  displayedGeneration = framework_units().generation();
  clear();
  setRowCount(0);
  const auto* definition = MaterialCardPresentation::find(property);
  if (definition == nullptr || material == nullptr) {
    setColumnCount(0);
    updating = false;
    return;
  }
  if (material->hasPropertyError(property)) {
    PRINT_ERROR("Material display units changed. Invalid table input was discarded; enter the values again.\n");
    material->clearPropertyError(property);
  }
  setColumnCount(static_cast<int>(definition->columns.size()));
  setHorizontalHeaderLabels(MaterialCardPresentation::columns(property, material->workspace));
  for (const auto& row : material->getMatrixPropertyGUI(property)) {
    insertNewRow(row);
  }
  insertLastRow();
  updating = false;
}
std::vector<std::vector<double>> MaterialManagementTable::getMatrixPropertyGUI() {
  std::vector<std::vector<double>> rows;
  const auto* definition = MaterialCardPresentation::find(property);
  if (material == nullptr || definition == nullptr) {
    return rows;
  }
  for (int row = 0; row < rowCount() - 1; ++row) {
    std::vector<double> values;
    for (int column = 0; column < columnCount(); ++column) {
      const auto* cell = item(row, column);
      QString text;
      if (cell != nullptr) {
        text = cell->text().trimmed();
      }
      if (text.isEmpty() && definition->columns[static_cast<std::size_t>(column)].optional && column == columnCount() - 1) {
        break;
      }
      bool valid = false;
      double value = text.toDouble(&valid);
      // Formatting is for display only. Editing another cell must not round
      // the original value loaded into this cell.
      if (cell != nullptr && cell->data(Qt::UserRole).isValid() && text == cell->data(Qt::UserRole + 1).toString())
      {
        value = cell->data(Qt::UserRole).toDouble(&valid);
      }
      if (!valid || !std::isfinite(value)) {
        const std::string error = "Material row " + std::to_string(row + 1) + " requires a finite value for " + definition->columns[static_cast<std::size_t>(column)].title + ".";
        material->setPropertyError(property, error);
        PRINT_ERROR("%s\n", error.c_str());
        return {};
      }
      values.push_back(value);
    }
    rows.push_back(std::move(values));
  }
  material->clearPropertyError(property);
  return rows;
}
void MaterialManagementTable::insertNewRow(std::vector<double> values) {
  const int row = rowCount();
  insertRow(row);
  setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
  for (int column = 0; column < columnCount(); ++column) {
    auto* cell = new QTableWidgetItem();
    if (static_cast<std::size_t>(column) < values.size())
    {
      const double value = values[static_cast<std::size_t>(column)];
      const QString text = QString::number(value, 'g', 15);
      cell->setText(text);
      cell->setData(Qt::UserRole, value);
      cell->setData(Qt::UserRole + 1, text);
    }
    setItem(row, column, cell);
  }
}
void MaterialManagementTable::insertLastRow() {
  insertNewRow({});
  setVerticalHeaderItem(rowCount() - 1, new QTableWidgetItem("*"));
}
bool MaterialManagementTable::checkRowEmtpy(int row) {
  for (int column = 0; column < columnCount(); ++column) {
    const auto* cell = item(row, column);
    if (cell != nullptr && !cell->text().trimmed().isEmpty()) {
      return false;
    }
  }
  return true;
}
void MaterialManagementTable::cellChanged(int row, int column) {
  if (updating || material == nullptr) {
    return;
  }
  if (displayedGeneration != framework_units().generation()) {
    PRINT_ERROR("Material display units changed. The table has been refreshed; enter the value again.\n");
    update(material);
    return;
  }
  const QSignalBlocker blocker(this);
  if (row == rowCount() - 1 && !checkRowEmtpy(row)) {
    insertLastRow();
    setCurrentCell(row, column);
  } else if (row < rowCount() - 1 && checkRowEmtpy(row)) {
    removeRow(row);
  }
  for (int index = 0; index < rowCount() - 1; ++index) {
    setVerticalHeaderItem(index, new QTableWidgetItem(QString::number(index + 1)));
  }
  const auto values = getMatrixPropertyGUI();
  if (!material->hasPropertyError(property)) {
    material->setMatrixPropertyGUI(property, values);
  }
}
