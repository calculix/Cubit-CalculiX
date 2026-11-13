#ifndef POSTPROCESSORWORKSPACE_HPP
#define POSTPROCESSORWORKSPACE_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QWidget>
#include <QPointer>
#include <functional>
#include <array>
#include <memory>
#include <optional>
#include <cstdint>
#include <cstddef>
#include "Framework/Results/ResultJobEntry.hpp"

class PostprocessorApplicationInterface;
class PostprocessorAnnotationControls;
class PostprocessorResultsTree;
class PostprocessorProperties;
class PostprocessorTableView;
class QSplitter;
class QTabWidget;
class PostprocessorPlotPane;
class QLabel;
class PostprocessorOptionsPanel;
class PostprocessorMouseOptionsPanel;
class PostprocessorViewport;
class PostprocessorFrameNavigation;
class PostprocessorColorControls;
class PostprocessorDeformationControls;
class PostprocessorPickControls;
class PostprocessorToolbar;
class PostprocessorSourceControls;
class Claro;
class QMainWindow;
namespace Postprocessor
{
  enum class DatasetVisibilityAction;
  enum class CaptureTarget;
}

class PostprocessorWorkspace : public QWidget
{
public:
  PostprocessorWorkspace(QMainWindow* host,
    std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent);
  ~PostprocessorWorkspace() override;
  std::array<int,2> capture_reference_size(Postprocessor::CaptureTarget target,const std::string& id) const;
  void refresh();
  bool bind_native_toolbar(Claro* host, std::string& error);
  bool camera_command_active() const noexcept;
  void workspace_view(WorkspaceView& view);

private:
  void refresh_result_panes(const std::shared_ptr<PostprocessorApplicationInterface>& application,bool dataChanged);
  void submit(const QString& command);
  void capture_dialog(bool report,bool quick);
  void import_results();
  void add_job(const SimResults::ResultJobEntry& job);
  QString frame_command(std::size_t frame) const;
  void source_command(const QString& verb, const QString& name);
  void dataset_command(const QString& source, Postprocessor::DatasetVisibilityAction action, std::optional<std::size_t> dataset);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  PostprocessorResultsTree* tree = nullptr;
  PostprocessorProperties* properties = nullptr;
  PostprocessorViewport* viewport = nullptr;
  QSplitter* viewSplitter = nullptr;
  QTabWidget* resultTabs = nullptr;
  PostprocessorPlotPane* plotPane = nullptr;
  std::optional<std::uint64_t> panelPlotRevision;
  bool panelPlotActive = false;
  bool tablePaneInitialized = false;
  PostprocessorTableView* tableView = nullptr;
  QLabel* viewTitle = nullptr;
  PostprocessorFrameNavigation* navigation = nullptr;
  PostprocessorDeformationControls* deformationControls = nullptr;
  PostprocessorPickControls* pickControls = nullptr;
  PostprocessorAnnotationControls* annotationControls = nullptr;
  QPointer<PostprocessorOptionsPanel> optionsPanel;
  QPointer<PostprocessorMouseOptionsPanel> mouseOptionsPanel;
  PostprocessorColorControls* colorControls = nullptr;
  QToolBar* navigationToolbar = nullptr;
  QToolBar* fieldToolbar = nullptr;
  QToolBar* deformationToolbar = nullptr;
  QDockWidget* resultsDock = nullptr;
  QDockWidget* propertiesDock = nullptr;
  QToolBar* toolbar = nullptr;
  PostprocessorToolbar* nativeToolbar = nullptr;
  QToolBar* sourceToolbar = nullptr;
  PostprocessorSourceControls* sourceControls = nullptr;
  QList<QAction*> menus;
  QAction* closeSelected = nullptr;
  QAction* closeAll = nullptr;
  std::optional<std::uint64_t> panelRevision;
  std::optional<std::uint64_t> panelSelectionRevision;
  std::string panelSource;
  std::string panelView;
};

#endif // POSTPROCESSORWORKSPACE_HPP
