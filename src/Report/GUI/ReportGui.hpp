#pragma once
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QPointer>
#include <memory>
#include <functional>
class ReportApplicationInterface;
class ReportWorkspace;
class ReportGuiObserver;

class ReportGui
{
public:
  ReportGui(std::weak_ptr<ReportApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent, std::string component_directory);
  ~ReportGui();
  bool start(QWidget* host, std::string& error);
  bool workspace_view(WorkspaceView& view, std::string& error);

private:
  std::string componentDirectory;
  std::weak_ptr<ReportApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submit;
  std::function<bool(const std::string&, std::string&)> silent;
  QPointer<ReportWorkspace> workspace;
  std::unique_ptr<ReportGuiObserver> observer;
};
