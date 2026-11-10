#ifndef POSTPROCESSOR_TABLEVIEW_HPP
#define POSTPROCESSOR_TABLEVIEW_HPP
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <QWidget>
class PostprocessorTableModel;
class QLabel;
class PostprocessorTableView : public QWidget
{
public:
  explicit PostprocessorTableView(QWidget* parent);
  void show_data(Postprocessor::ResultTableData data);
private:
  PostprocessorTableModel* model;
  QLabel* title;
};
#endif
