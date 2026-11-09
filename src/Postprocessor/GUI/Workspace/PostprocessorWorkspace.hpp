#ifndef POSTPROCESSORWORKSPACE_HPP
#define POSTPROCESSORWORKSPACE_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QWidget>
#include <QPointer>
#include <functional>
#include <memory>
#include <optional>
#include <cstdint>
#include <cstddef>

class PostprocessorApplicationInterface;
class PostprocessorResultsTree;
class PostprocessorProperties;
class PostprocessorOptionsPanel;
class PostprocessorMouseOptionsPanel;
class PostprocessorViewport;
class PostprocessorFrameNavigation;
class PostprocessorColorControls;
class PostprocessorDeformationControls;
class PostprocessorPickControls;
class QMainWindow;
namespace Postprocessor
{
  enum class DatasetVisibilityAction;
}

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
  QString frame_command(std::size_t frame) const;
  void source_command(const QString& verb, const QString& name);
  void dataset_command(const QString& source, Postprocessor::DatasetVisibilityAction action, std::optional<std::size_t> dataset);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  PostprocessorResultsTree* tree = nullptr;
  PostprocessorProperties* properties = nullptr;
  PostprocessorViewport* viewport = nullptr;
  PostprocessorFrameNavigation* navigation = nullptr;
  PostprocessorDeformationControls* deformationControls = nullptr;
  PostprocessorPickControls* pickControls = nullptr;
  QPointer<PostprocessorOptionsPanel> optionsPanel;
  QPointer<PostprocessorMouseOptionsPanel> mouseOptionsPanel;
  PostprocessorColorControls* colorControls = nullptr;
  QToolBar* navigationToolbar = nullptr;
  QToolBar* fieldToolbar = nullptr;
  QToolBar* deformationToolbar = nullptr;
  QDockWidget* resultsDock = nullptr;
  QDockWidget* propertiesDock = nullptr;
  QToolBar* toolbar = nullptr;
  QList<QAction*> menus;
  QAction* closeSelected = nullptr;
  QAction* closeAll = nullptr;
  std::optional<std::uint64_t> panelRevision;
  std::optional<std::uint64_t> panelSelectionRevision;
  std::string panelSource;
  std::string panelView;
};

#endif // POSTPROCESSORWORKSPACE_HPP
