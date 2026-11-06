#ifndef POSTPROCESSORGUI_HPP
#define POSTPROCESSORGUI_HPP

#include <QPointer>
#include <functional>
#include <memory>
#include <string>

class QWidget;
class PostprocessorWorkspace;
class PostprocessorApplicationInterface;
class PostprocessorGuiObserver;
struct WorkspaceView;

class PostprocessorGui
{
public:
  PostprocessorGui(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit);
  ~PostprocessorGui();
  bool start(QWidget* host, std::string& error);
  void stop() noexcept;
  void refresh();
  bool workspace_view(WorkspaceView& view, std::string& error);

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  QPointer<PostprocessorWorkspace> workspace;
  std::unique_ptr<PostprocessorGuiObserver> observer;
};

#endif // POSTPROCESSORGUI_HPP
