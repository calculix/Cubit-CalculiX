#include "PostprocessorGui.hpp"
#include "Workspace/PostprocessorWorkspace.hpp"
#include "Observers/PostprocessorGuiObserver.hpp"
#include "CubitMessage.hpp"

#include <QMainWindow>
#include <QThread>
#include <exception>
#include <utility>

PostprocessorGui::PostprocessorGui(std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&, std::string&)> submit)
  : application(std::move(application)), submitCommand(std::move(submit))
{
}

PostprocessorGui::~PostprocessorGui()
{
  stop();
}

bool PostprocessorGui::start(QWidget* host, std::string& error)
{
  error.clear();
  auto* window = qobject_cast<QMainWindow*>(host);

  if (window == nullptr || QThread::currentThread() != window->thread() || application.expired())
  {
    error = "Postprocessor GUI startup requires its application and the Cubit GUI thread.";
    return false;
  }

  if (workspace != nullptr)
  {
    return true;
  }

  workspace = new PostprocessorWorkspace(window, application, submitCommand);
  observer = std::make_unique<PostprocessorGuiObserver>([this]()
  {
    refresh();
  });
  return true;
}

void PostprocessorGui::stop() noexcept
{
  observer.reset();
  delete workspace.data();
  workspace = nullptr;
}

void PostprocessorGui::refresh()
{
  try
  {
    if (workspace != nullptr)
    {
      workspace->refresh();
    }
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot refresh Postprocessor panels: %s\n", exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot refresh Postprocessor panels: unknown failure.\n");
  }
}

bool PostprocessorGui::workspace_view(WorkspaceView& view, std::string& error)
{
  error.clear();

  if (workspace == nullptr)
  {
    error = "The Postprocessor workspace widgets are unavailable.";
    return false;
  }

  workspace->workspace_view(view);
  return true;
}
