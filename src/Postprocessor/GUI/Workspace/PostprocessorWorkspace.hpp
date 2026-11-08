#ifndef POSTPROCESSORWORKSPACE_HPP
#define POSTPROCESSORWORKSPACE_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QWidget>
#include <QPointer>
#include <functional>
#include <memory>
#include <optional>
#include <cstdint>

class PostprocessorApplicationInterface;
class PostprocessorResultsTree;
class PostprocessorProperties;
class PostprocessorOptionsPanel;
class PostprocessorMouseOptionsPanel;
class PostprocessorViewport;
class QMainWindow;

class PostprocessorWorkspace : public QWidget
{
public:
  PostprocessorWorkspace(QMainWindow* host,
    std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent);
  ~PostprocessorWorkspace() override;
  void refresh();
  bool camera_command_active() const noexcept;
  void workspace_view(WorkspaceView& view);

private:
  void submit(const QString& command);
  void source_command(const QString& verb, const QString& name);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  PostprocessorResultsTree* tree = nullptr;
  PostprocessorProperties* properties = nullptr;
  PostprocessorViewport* viewport = nullptr;
  QPointer<PostprocessorOptionsPanel> optionsPanel;
  QPointer<PostprocessorMouseOptionsPanel> mouseOptionsPanel;
  QDockWidget* resultsDock = nullptr;
  QDockWidget* propertiesDock = nullptr;
  QToolBar* toolbar = nullptr;
  QList<QAction*> menus;
  QAction* closeSelected = nullptr;
  QAction* closeAll = nullptr;
  std::optional<std::uint64_t> panelRevision;
  std::string panelSource;
};

#endif // POSTPROCESSORWORKSPACE_HPP
