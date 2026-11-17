#ifndef POSTPROCESSOR_TABLEVIEW_HPP
#define POSTPROCESSOR_TABLEVIEW_HPP
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <QWidget>
#include <QString>
#include <QPoint>
#include <QSet>
#include <functional>
#include <cstdint>
class PostprocessorTableModel;
class QLabel;
class QTableView;
class PostprocessorTableView : public QWidget
{
public:
  using Plot = std::function<void(const Postprocessor::TableReference&,const Postprocessor::ResultTableData&,std::size_t,std::size_t)>;
  PostprocessorTableView(std::function<void()> closeTable, QWidget* parent, Plot plot = {},std::function<void(const QString&)> submit = {});
  void set_plot_reference(const std::optional<Postprocessor::TableReference>& reference);
  void show_data(Postprocessor::ResultTableData data);
private:
  void show_context_menu(const QPoint& globalPosition,int column,std::optional<std::size_t> sourceRow,
    const Plot& plot,const std::function<void(const QString&)>& submit);
  void resize_columns_to_contents();
  void widen_columns_for_visible_rows();
  void measure_rows(const std::vector<int>& rows, bool reset);
  void queue_column_fit();
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  PostprocessorTableModel* model;
  QLabel* title;
  QTableView* table;
  Postprocessor::ResultTableData shown;
  std::optional<Postprocessor::TableReference> plotReference;
  QSet<int> manualColumns;
  std::optional<std::uint64_t> shownUnitGeneration;
  bool resizingColumns = false;
  bool fitQueued = false;
};
#endif
