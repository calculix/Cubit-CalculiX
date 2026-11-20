#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CalculiXUnitPresentation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Core/units/UnitLabelFormat.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "Core/units/CalculiXDofUnits.hpp"
#include <QApplication>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QStringList>
#include <QTableWidget>
#include <QVariant>
#include <QWidget>
#include <cmath>
#include <functional>
#include <utility>

namespace
{
bool convert_text(const QString& text, const std::string& from, const std::string& to,
  const SimUnits::QuantityUnit& quantity, QString& output, std::string& error)
{
  output = text;
  if (from == to || text.trimmed().isEmpty() || text.trimmed().compare("free", Qt::CaseInsensitive) == 0 ||
      text.trimmed().compare("fixed", Qt::CaseInsensitive) == 0)
  {
    return true;
  }
  QStringList converted;
  const auto tokens = text.simplified().split(' ', Qt::SkipEmptyParts);
  for (const auto& token : tokens)
  {
    bool valid = false;
    const double value = QLocale::c().toDouble(token, &valid);
    double canonical = 0.0;
    double displayed = 0.0;
    if (!valid || !std::isfinite(value))
    {
      error = "Finish the numeric draft in its displayed units before applying the changed unit preferences.";
      return false;
    }
    if (!framework_units().to_si(value, from, quantity.dimension, quantity.role, canonical, error) ||
        !framework_units().from_si(canonical, to, quantity.dimension, quantity.role, displayed, error))
    {
      return false;
    }
    converted.push_back(QString::number(displayed, 'g', 17));
  }
  output = converted.join(' ');
  return true;
}
}

CalculiXUnitPresentation::CalculiXUnitPresentation(QWidget* owner) : QObject(owner), panel(owner)
{
  owner->installEventFilter(this);
}

void CalculiXUnitPresentation::bind(QLabel& label, QLineEdit& field, const QString& title, const std::string& quantity)
{
  Binding binding;
  binding.label = &label;
  binding.field = &field;
  binding.title = title;
  binding.quantity = quantity;
  std::string error;
  framework_units().display_unit("calculix", quantity, binding.unit, error);
  bindings.push_back(std::move(binding));
  label.setMaximumWidth(QWIDGETSIZE_MAX);
  connect(&field, &QLineEdit::textEdited, this, [this]
  {
    std::string error;
    refresh(error);
  });
  refresh(error);
}

void CalculiXUnitPresentation::bind_table(QTableWidget& table, int column, const std::string& quantity)
{
  for (const auto& existing : bindings)
  {
    if (existing.table == &table && existing.column == column)
    {
      return;
    }
  }
  Binding binding;
  binding.table = &table;
  binding.column = column;
  binding.quantity = quantity;
  if (const auto* header = table.horizontalHeaderItem(column))
  {
    binding.title = header->text();
  }
  std::string error;
  framework_units().display_unit("calculix", quantity, binding.unit, error);
  bindings.push_back(std::move(binding));
  connect(&table, &QTableWidget::itemChanged, this, [this, &table]
  {
    auto* focus = QApplication::focusWidget();
    if (blocked && focus != nullptr && (&table == focus || table.isAncestorOf(focus)))
    {
      std::string error;
      refresh(error);
    }
  });
  refresh(error);
}

void CalculiXUnitPresentation::set_quantity(QLineEdit& field, const std::string& quantity)
{
  for (auto& binding : bindings)
  {
    if (binding.field == &field)
    {
      binding.quantity = quantity;
      std::string error;
      framework_units().display_unit("calculix", quantity, binding.unit, error);
    }
  }
  std::string error;
  refresh(error);
}

void CalculiXUnitPresentation::bind_equations(QTableWidget& table, int node_column, int dof_column, int coefficient_column)
{
  bind_table(table, coefficient_column, "length");
  auto& binding = bindings.back();
  const QPointer<QTableWidget> target(&table);
  binding.equation_quantity = [target, node_column, dof_column](int row) -> std::string
  {
    if (target == nullptr || target->item(row, node_column) == nullptr || target->item(row, dof_column) == nullptr)
    {
      return {};
    }
    bool validNode = false;
    bool validDof = false;
    const int node = target->item(row, node_column)->text().toInt(&validNode);
    const int dof = target->item(row, dof_column)->text().toInt(&validDof);
    std::set<int> rotationNodes;
    std::string error;
    std::string quantity;
    if (!validNode || node <= 0 || !validDof ||
        !CalculiXConnectionsInterface().get_rotation_node_ids(rotationNodes, error) ||
        !CalculiXDofUnits::equation_quantity(dof, rotationNodes.count(node) != 0, quantity, error))
    {
      return {};
    }
    return quantity;
  };
  binding.row_units.clear();
  connect(&table, &QTableWidget::itemChanged, this, [this, &table, node_column, dof_column, coefficient_column](QTableWidgetItem* item)
  {
    auto* focus = QApplication::focusWidget();
    if (refreshing || item == nullptr || focus == nullptr ||
        (&table != focus && !table.isAncestorOf(focus)) ||
        (item->column() != node_column && item->column() != dof_column))
    {
      return;
    }
    // Selecting a different node/DOF explicitly changes the coefficient's
    // dimension. Its edited numeric value now belongs to that selected role.
    for (auto& entry : bindings)
    {
      if (entry.table == &table && entry.column == coefficient_column)
      {
        entry.row_units.erase(item->row());
      }
    }
    std::string error;
    refresh(error);
  });
  std::string error;
  refresh(error);
}

void CalculiXUnitPresentation::set_table_quantity(QTableWidget& table, int column, const std::string& quantity)
{
  for (auto& binding : bindings)
  {
    if (binding.table == &table && binding.column == column)
    {
      binding.quantity = quantity;
      if (quantity == "frequency" && binding.title == "Time")
      {
        binding.title = "Frequency";
      }
      else if (quantity == "time" && binding.title == "Frequency")
      {
        binding.title = "Time";
      }
      std::string error;
      framework_units().display_unit("calculix", quantity, binding.unit, error);
    }
  }
  std::string error;
  refresh(error);
}

void CalculiXUnitPresentation::block_apply(bool value)
{
  if (panel == nullptr)
  {
    return;
  }
  for (auto* button : panel->findChildren<QPushButton*>())
  {
    if (button->text() != "Apply")
    {
      continue;
    }
    if (value)
    {
      if (!blocked)
      {
        button->setProperty("calculixUnitsWasEnabled", button->isEnabled());
      }
      button->setEnabled(false);
    }
    else if (blocked)
    {
      button->setEnabled(button->property("calculixUnitsWasEnabled").toBool());
    }
  }
  blocked = value;
}

bool CalculiXUnitPresentation::refresh(std::string& error)
{
  error.clear();
  if (refreshing)
  {
    return true;
  }
  QScopedValueRollback<bool> guard(refreshing, true);
  // Stage ALL numeric fields/cells first, so a malformed draft cannot leave a
  // panel with partially converted values and partially changed labels.
  std::vector<std::function<void()>> edits;
  std::vector<std::string> units;
  for (auto& binding : bindings)
  {
    SimUnits::QuantityUnit quantity;
    std::string unit;
    if (!framework_units().quantity(binding.quantity, quantity, error) ||
        !framework_units().display_unit("calculix", binding.quantity, unit, error))
    {
      block_apply(true);
      return false;
    }
    units.push_back(unit);
    if (binding.field != nullptr)
    {
      QString text;
      if (!convert_text(binding.field->text(), binding.unit, unit, quantity, text, error))
      {
        binding.field->setToolTip(QString::fromStdString(error));
        block_apply(true);
        return false;
      }
      const auto field = binding.field;
      edits.push_back([field, text]
      {
        if (field != nullptr)
        {
          QSignalBlocker blocker(field.data());
          field->setText(text);
        }
      });
    }
    if (binding.table != nullptr)
    {
      for (int row = 0; row < binding.table->rowCount(); ++row)
      {
        auto* item = binding.table->item(row, binding.column);
        if (item == nullptr)
        {
          continue;
        }
        QString text;
        std::string from = binding.unit;
        std::string to = unit;
        auto cellQuantity = quantity;
        if (binding.equation_quantity && !item->text().trimmed().isEmpty())
        {
          const auto id = binding.equation_quantity(row);
          std::string baseUnit;
          if (id.empty() || !framework_units().quantity(id, cellQuantity, error) ||
              !framework_units().display_unit("calculix", id, baseUnit, error))
          {
            error = "Complete the equation node and DOF before converting its coefficient.";
            block_apply(true);
            return false;
          }
          for (double& exponent : cellQuantity.dimension)
          {
            exponent = -exponent;
          }
          cellQuantity.role = SimUnits::UnitRole::Linear;
          to = "1/(" + baseUnit + ")";
          from = to;
          const auto old = binding.row_units.find(row);
          if (old != binding.row_units.end())
          {
            from = old->second;
          }
          auto* destination = &binding;
          edits.push_back([destination, row, to]
          {
            destination->row_units[row] = to;
          });
        }
        if (!convert_text(item->text(), from, to, cellQuantity, text, error))
        {
          binding.table->setToolTip(QString::fromStdString(error));
          block_apply(true);
          return false;
        }
        const auto table = binding.table;
        const int column = binding.column;
        const QString cellTooltip = "Input unit: " + QString::fromStdString(to);
        edits.push_back([table, row, column, text, cellTooltip]
        {
          if (table != nullptr && table->item(row, column) != nullptr)
          {
            QSignalBlocker blocker(table.data());
            table->item(row, column)->setText(text);
            table->item(row, column)->setToolTip(cellTooltip);
          }
        });
      }
    }
  }
  for (const auto& edit : edits)
  {
    edit();
  }
  for (std::size_t index = 0; index < bindings.size(); ++index)
  {
    auto& binding = bindings[index];
    binding.unit = units[index];
    QString title = binding.title + " [" + UnitLabelFormat::format(binding.unit) + "]";
    if (binding.equation_quantity)
    {
      title = binding.title + " [inverse DOF unit]";
    }
    QString tooltip = "Input unit: " + QString::fromStdString(binding.unit) + ". Values are stored in canonical SI units.";
    if (binding.equation_quantity)
    {
      tooltip = "Each coefficient uses the inverse unit of its actual node/DOF. The current input unit is shown in each cell tooltip.";
    }
    if (binding.label != nullptr)
    {
      binding.label->setText(title);
      binding.label->setToolTip(tooltip);
    }
    if (binding.field != nullptr)
    {
      binding.field->setToolTip(tooltip);
    }
    if (binding.table != nullptr)
    {
      auto* header = binding.table->horizontalHeaderItem(binding.column);
      if (header != nullptr)
      {
        header->setText(title);
        header->setToolTip(tooltip);
      }
    }
  }
  block_apply(false);
  return true;
}

bool CalculiXUnitPresentation::refresh_panel(QWidget& panel, std::string& error)
{
  bool success = true;
  for (auto* presentation : panel.findChildren<CalculiXUnitPresentation*>())
  {
    std::string message;
    if (!presentation->refresh(message))
    {
      error = message;
      success = false;
    }
  }
  return success;
}

QString CalculiXUnitPresentation::title(const QString& text, const std::string& quantity)
{
  std::string unit;
  std::string error;
  if (!framework_units().display_unit("calculix", quantity, unit, error))
  {
    return text + " [unknown unit]";
  }
  return text + " [" + UnitLabelFormat::format(unit) + "]";
}

void CalculiXUnitPresentation::accept_loaded_values(QWidget& panel)
{
  for (auto* presentation : panel.findChildren<CalculiXUnitPresentation*>())
  {
    for (auto& binding : presentation->bindings)
    {
      binding.row_units.clear();
      std::string error;
      framework_units().display_unit("calculix", binding.quantity, binding.unit, error);
    }
    std::string error;
    presentation->refresh(error);
  }
}

bool CalculiXUnitPresentation::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::Show)
  {
    std::string error;
    refresh(error);
  }
  return QObject::eventFilter(watched, event);
}
