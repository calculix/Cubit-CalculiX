#ifndef POSTPROCESSORRESULTSTREE_HPP
#define POSTPROCESSORRESULTSTREE_HPP

#include "Postprocessor/Core/session/PostprocessorSession.hpp"
#include <QTreeWidget>
#include <functional>

class PostprocessorResultsTree : public QTreeWidget
{
public:
  PostprocessorResultsTree(std::function<void(const QString&)> select,
    std::function<void(const QString&, bool)> visibility, QWidget* parent = nullptr);
  void refresh(const Postprocessor::PostprocessorSession& session,
    const std::function<bool(const std::string&)>& has_mesh);
};

#endif // POSTPROCESSORRESULTSTREE_HPP
