#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <string>
#include <vector>
#include <functional>
#include <map>

class QLabel;
class QLineEdit;
class QTableWidget;
class QWidget;

// Unit presentation and draft conversion only; panels retain their own widgets,
// layouts, validation and commands. No model values are changed here.
class CalculiXUnitPresentation : public QObject
{
  Q_OBJECT
public:
  explicit CalculiXUnitPresentation(QWidget* panel);
  void bind(QLabel& label, QLineEdit& field, const QString& title, const std::string& quantity);
  void bind_table(QTableWidget& table, int column, const std::string& quantity);
  void bind_equations(QTableWidget& table, int node_column, int dof_column, int coefficient_column);
  void set_quantity(QLineEdit& field, const std::string& quantity);
  void set_table_quantity(QTableWidget& table, int column, const std::string& quantity);
  bool refresh(std::string& error);
  static bool refresh_panel(QWidget& panel, std::string& error);
  static QString title(const QString& text, const std::string& quantity);
  // Call after a settings query populates fields in the CURRENT display units.
  static void accept_loaded_values(QWidget& panel);

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  struct Binding
  {
    QPointer<QLabel> label;
    QPointer<QLineEdit> field;
    QPointer<QTableWidget> table;
    int column = -1;
    QString title;
    std::string quantity;
    std::string unit;
    std::function<std::string(int)> equation_quantity;
    std::map<int, std::string> row_units;
  };
  void block_apply(bool blocked);
  QPointer<QWidget> panel;
  std::vector<Binding> bindings;
  bool refreshing = false;
  bool blocked = false;
};
