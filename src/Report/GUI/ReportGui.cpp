#include "ReportGui.hpp"
#include "ReportIconLoader.hpp"
#include <utility>
#include "ReportWorkspace.hpp"
#include "ReportGuiObserver.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QMainWindow>
#include <QThread>
#include <exception>

ReportGui::ReportGui(std::weak_ptr<ReportApplicationInterface> active,
  std::function<bool(const std::string&, std::string&)> send,
  std::function<bool(const std::string&, std::string&)> quiet, std::string directory)
  : componentDirectory(std::move(directory)), application(std::move(active)), submit(std::move(send)), silent(std::move(quiet))
{
}

ReportGui::~ReportGui()
{
  observer.reset();
  if (const auto active = application.lock())
  {
    active->set_gui_available(false);
  }
  delete workspace.data();
}

bool ReportGui::start(QWidget* host, std::string& error)
{
  auto* window = qobject_cast<QMainWindow*>(host);
  const auto active = application.lock();
  if (!window || !active || QThread::currentThread() != window->thread())
  {
    error = "Report GUI startup requires its application and the Cubit GUI thread.";
    return false;
  }
  workspace = new ReportWorkspace(host, application, submit, silent, ReportIconLoader::load(componentDirectory));
  active->set_gui_available(true);
  observer = std::make_unique<ReportGuiObserver>([this]()
  {
    try
    {
      if (workspace)
      {
        workspace->refresh();
      }
    }
    catch (const std::exception& exception)
    {
      PRINT_ERROR("Report GUI refresh failed: %s\n", exception.what());
    }
    catch (...)
    {
      PRINT_ERROR("Report GUI refresh failed.\n");
    }
  });
  error.clear();
  return true;
}

bool ReportGui::workspace_view(WorkspaceView& view, std::string& error)
{
  if (!workspace)
  {
    error = "Report GUI is not started.";
    return false;
  }
  workspace->workspace_view(view);
  error.clear();
  return true;
}
