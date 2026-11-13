#ifndef POSTPROCESSORANNOTATIONMENU_HPP
#define POSTPROCESSORANNOTATIONMENU_HPP
#include "Postprocessor/Core/selection/EntityReference.hpp"
#include <QString>
#include <QPoint>
#include <functional>
class QWidget;
class PostprocessorApplicationInterface;
QString annotation_quote(const std::string& value);
QString annotation_add_command(const PostprocessorApplicationInterface& application,
  const std::string& view, const Postprocessor::EntityReference& entity);
void show_annotation_menu(QWidget* parent, const QPoint& globalPosition,
  const PostprocessorApplicationInterface& application, const std::string& view,
  const std::string& label, const std::optional<Postprocessor::EntityReference>& entity,
  const std::function<void(const QString&)>& submit);
#endif
