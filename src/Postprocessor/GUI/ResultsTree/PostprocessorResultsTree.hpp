#ifndef POSTPROCESSORRESULTSTREE_HPP
#define POSTPROCESSORRESULTSTREE_HPP

#include "Postprocessor/Core/session/PostprocessorSession.hpp"
#include <QTreeWidget>
#include <functional>

class PostprocessorResultsTree : public QTreeWidget
{
public:
  explicit PostprocessorResultsTree(std::function<void(const QString&)> select, QWidget* parent = nullptr);
  void refresh(const Postprocessor::PostprocessorSession& session);
};

#endif // POSTPROCESSORRESULTSTREE_HPP
