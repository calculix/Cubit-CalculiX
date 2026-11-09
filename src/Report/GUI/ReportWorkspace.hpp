#pragma once
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include "Framework/Reporting/ReportSourceEntry.hpp"
#include <QWidget>
#include <QPointer>
#include <QIcon>
#include <functional>
#include <memory>
#include <cstdint>
class ReportApplicationInterface;
class ReportContentTree;
class ReportPreviewWidget;
class ReportPreviewWindow;
class QComboBox;
class QSpinBox;
class QSlider;
class QPushButton;
class QLabel;
class QMenu;

class ReportWorkspace final : public QWidget
{
public:
  ReportWorkspace(QWidget* host, std::weak_ptr<ReportApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent, const QIcon& icon);
  ~ReportWorkspace() override;
  void refresh();
  void workspace_view(WorkspaceView& view);

private:
  void dispatch(const std::string& command, bool quiet = false);
  void profile_action(const std::string& action);
  void choose_source(int index);
  void generate();
  std::weak_ptr<ReportApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submit;
  std::function<bool(const std::string&, std::string&)> silent;
  QIcon icon;
  QPointer<QDockWidget> treeDock;
  ReportContentTree* tree;
  ReportPreviewWidget* page;
  ReportPreviewWindow* preview;
  QComboBox* sources;
  QComboBox* reports;
  QComboBox* profiles;
  QWidget* navigationBar;
  QSpinBox* pageNumber;
  QSlider* pageSlider;
  QSpinBox* zoom;
  QPushButton* generateButton;
  QPushButton* updateButton;
  QPushButton* settingsButton;
  QPushButton* profileButton;
  QLabel* status;
  QMenu* menu;
  std::vector<ReportSourceEntry> entries;
  std::string profileWorkspace;
  std::uint64_t previewWindowRevision = 0;
  std::uint64_t pageRevision = 0;
  std::uint64_t fitRevision = 0;
};
