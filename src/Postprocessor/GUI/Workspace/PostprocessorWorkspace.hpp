#ifndef POSTPROCESSORWORKSPACE_HPP
#define POSTPROCESSORWORKSPACE_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QWidget>
#include <functional>
#include <memory>

class PostprocessorApplicationInterface;
class PostprocessorResultsTree;
class PostprocessorProperties;
class QMainWindow;

class PostprocessorWorkspace : public QWidget
{
public:
  PostprocessorWorkspace(QMainWindow* host,
    std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit);
  ~PostprocessorWorkspace() override;
  void refresh();
  void workspace_view(WorkspaceView& view);

private:
  void submit(const QString& command);
  void source_command(const QString& verb, const QString& name);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  PostprocessorResultsTree* tree = nullptr;
  PostprocessorProperties* properties = nullptr;
  QDockWidget* resultsDock = nullptr;
  QDockWidget* propertiesDock = nullptr;
  QToolBar* toolbar = nullptr;
  QList<QAction*> menus;
  QAction* closeSelected = nullptr;
  QAction* closeAll = nullptr;
};

#endif // POSTPROCESSORWORKSPACE_HPP
