#include "UnitsManagement.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Core/units/UnitLabelFormat.hpp"
#include "CubitMessage.hpp"
#include <QComboBox>
#include <QCloseEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <utility>

UnitsManagement::UnitsManagement(std::string workspace, std::string commandPrefix, Submit submit, QWidget* parent) : QDialog(parent), workspace(std::move(workspace)), prefix(std::move(commandPrefix)), submit(std::move(submit)) {
  setWindowTitle(tr("Workspace units"));
  resize(700, 520);
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(new QLabel(QString::fromStdString(this->workspace), this));
  auto* selection = new QHBoxLayout();
  systems = new QComboBox(this);
  for (const auto& id : framework_units().system_ids()) {
    systems->addItem(QString::fromStdString(id));
  }
  auto* apply = new QPushButton(tr("Apply system"), this);
  auto* save = new QPushButton(tr("Save as default"), this);
  selection->addWidget(systems);
  selection->addWidget(apply);
  selection->addWidget(save);
  layout->addLayout(selection);
  layout->addWidget(new QLabel(tr("Input/display preferences are independent of coherent solver units."), this));
  table = new QTableWidget(this);
  table->setColumnCount(3);
  table->setHorizontalHeaderLabels({tr("Quantity"), tr("Input/display unit"), tr("Coherent unit")});
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  layout->addWidget(table);
  auto* preferences = new QHBoxLayout();
  quantities = new QComboBox(this);
  expression = new QLineEdit(this);
  auto* set = new QPushButton(tr("Set preference"), this);
  auto* reset = new QPushButton(tr("Use coherent unit"), this);
  for (const auto& id : framework_units().quantity_ids()) {
    quantities->addItem(QString::fromStdString(id));
  }
  preferences->addWidget(quantities);
  preferences->addWidget(expression);
  preferences->addWidget(set);
  preferences->addWidget(reset);
  layout->addLayout(preferences);
  connect(apply, &QPushButton::clicked, this, [this]() {
    submit_command(prefix + " units system \"" + systems->currentText().toStdString() + "\"");
  });
  connect(save, &QPushButton::clicked, this, [this]() {
    submit_command(prefix + " units defaults save");
  });
  connect(set, &QPushButton::clicked, this, [this]() {
    const auto unit = expression->text().trimmed().toStdString();
    SimUnits::UnitDefinition checked;
    std::string error;
    if (!framework_units().unit(unit, checked, error)) {
      PRINT_ERROR("Cannot set unit preference: %s\n", error.c_str());
      return;
    }
    submit_command(prefix + " units display quantity \"" + quantities->currentText().toStdString() + "\" unit \"" + unit + "\"");
  });
  connect(reset, &QPushButton::clicked, this, [this]() {
    submit_command(prefix + " units display reset quantity \"" + quantities->currentText().toStdString() + "\"");
  });
  connect(quantities, &QComboBox::currentTextChanged, this, [this](const QString& quantity) {
    std::string unit;
    std::string error;
    if (framework_units().display_unit(this->workspace, quantity.toStdString(), unit, error)) {
      expression->setText(QString::fromStdString(unit));
    } else {
      PRINT_ERROR("Cannot read unit preference: %s\n", error.c_str());
    }
  });
  refresh();
}
void UnitsManagement::submit_command(const std::string& command) {
  std::string error;
  if (!submit || !submit(command, error)) {
    if (error.empty()) {
      error = "Workspace command submission is unavailable.";
    }
    PRINT_ERROR("Cannot change workspace units: %s\n", error.c_str());
    return;
  }
  refresh();
}
bool UnitsManagement::close_from_command() {
  commandClose = true;
  const bool accepted = QDialog::close();
  commandClose = false;
  return accepted;
}
void UnitsManagement::closeEvent(QCloseEvent* event) {
  if (commandClose) {
    QDialog::closeEvent(event);
    return;
  }
  event->ignore();
  request_close();
}
void UnitsManagement::reject() {
  if (commandClose) {
    QDialog::reject();
    return;
  }
  request_close();
}
void UnitsManagement::request_close() {
  if (closePending) {
    return;
  }
  closePending = true;
  QTimer::singleShot(0, this, [this]() {
    closePending = false;
    std::string error;
    if (!submit || !submit(prefix + " units close", error)) {
      if (error.empty()) {
        error = "Workspace command submission is unavailable.";
      }
      PRINT_ERROR("Cannot close workspace units: %s\n", error.c_str());
    }
  });
}
void UnitsManagement::refresh() {
  SimUnits::WorkspaceUnitContext context;
  std::string error;
  if (!framework_units().workspace_context(workspace, context, error)) {
    PRINT_ERROR("Cannot read workspace units: %s\n", error.c_str());
    return;
  }
  systems->setCurrentText(QString::fromStdString(context.system));
  const auto ids = framework_units().quantity_ids();
  table->setRowCount(static_cast<int>(ids.size()));
  for (std::size_t index = 0; index < ids.size(); ++index) {
    std::string displayed;
    SimUnits::QuantityUnit coherent;
    if (!framework_units().display_unit(workspace, ids[index], displayed, error) || !framework_units().system_unit(context.system, ids[index], coherent, error)) {
      PRINT_ERROR("Cannot read quantity units: %s\n", error.c_str());
      return;
    }
    table->setItem(static_cast<int>(index), 0, new QTableWidgetItem(QString::fromStdString(ids[index])));
    auto* displayItem = new QTableWidgetItem(UnitLabelFormat::format(displayed));
    displayItem->setToolTip(QString::fromStdString(displayed));
    table->setItem(static_cast<int>(index), 1, displayItem);
    auto* solverItem = new QTableWidgetItem(UnitLabelFormat::format(coherent.unit));
    solverItem->setToolTip(QString::fromStdString(coherent.unit));
    table->setItem(static_cast<int>(index), 2, solverItem);
  }
  std::string displayed;
  if (framework_units().display_unit(workspace, quantities->currentText().toStdString(), displayed, error)) {
    expression->setText(QString::fromStdString(displayed));
  } else {
    PRINT_ERROR("Cannot read selected quantity unit: %s\n", error.c_str());
  }
}
