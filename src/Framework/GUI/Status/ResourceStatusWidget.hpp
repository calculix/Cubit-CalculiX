#ifndef CUBITSIM_RESOURCESTATUSWIDGET_HPP
#define CUBITSIM_RESOURCESTATUSWIDGET_HPP

#include "Framework/Core/Monitoring/ResourceMonitor.hpp"
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QElapsedTimer>
#include <QPointer>
#include <QWidget>

class FrameworkUserOptions;
class FrameworkOptionsPanel;
class ModuleManager;
class ResourceUsageButton;
class QStatusBar;
class QToolButton;
class QTimer;
class QDialog;
class QTreeWidget;

class ResourceStatusWidget : public QWidget
{
public:
  ResourceStatusWidget(QStatusBar* statusBar, ModuleManager& modules, FrameworkUserOptions& preferences);
  ~ResourceStatusWidget() override;
  WorkspaceView options_page() const;
  void apply_options();

private:
  void tick();
  void display_snapshot();
  void show_details();
  void clear_indicators(const QString& reason);
  ModuleManager& modules;
  FrameworkUserOptions& preferences;
  QPointer<FrameworkOptionsPanel> optionsPanel;
  QPointer<QDialog> details;
  QPointer<QTreeWidget> detailsTree;
  ResourceUsageButton* cpuIndicator = nullptr;
  ResourceUsageButton* memoryIndicator = nullptr;
  QToolButton* jobsButton = nullptr;
  QTimer* timer = nullptr;
  QElapsedTimer lastRequest;
  std::unique_ptr<ResourceMonitor> monitor;
  std::optional<ResourceSnapshot> latest;
};

#endif
