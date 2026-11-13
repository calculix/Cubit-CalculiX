#ifndef POSTPROCESSORPLOTCONTROLS_HPP
#define POSTPROCESSORPLOTCONTROLS_HPP
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include "Postprocessor/Core/selection/EntityReference.hpp"
#include <QString>
#include <functional>
class QMenu;
class QWidget;
class PostprocessorApplicationInterface;
void add_plot_context_actions(QMenu& menu,const PostprocessorApplicationInterface& application,
  const Postprocessor::EntityReference& entity,const std::function<void(const QString&)>& submit);
void plot_table_dialog(QWidget* parent,const PostprocessorApplicationInterface& application,
  const Postprocessor::TableReference& reference,const Postprocessor::ResultTableData& data,
  std::size_t row,std::size_t column,const std::function<void(const QString&)>& submit);
#endif
