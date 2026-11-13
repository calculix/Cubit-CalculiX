#ifndef POSTPROCESSORGUI_HPP
#define POSTPROCESSORGUI_HPP

#include <QPointer>
#include <functional>
#include <array>
#include <memory>
#include <string>

namespace Postprocessor
{
  enum class CaptureTarget;
}
class QWidget;
class PostprocessorWorkspace;
class PostprocessorApplicationInterface;
class PostprocessorGuiObserver;
struct WorkspaceView;

class PostprocessorGui
{
public:
  PostprocessorGui(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent);
  ~PostprocessorGui();
  bool start(QWidget* host, std::string& error);
  void stop() noexcept;
  std::array<int,2> capture_reference_size(Postprocessor::CaptureTarget target,const std::string& id) const;
  void refresh();
  bool workspace_view(WorkspaceView& view, std::string& error);

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  std::function<bool(const std::string&, std::string&)> silentCommand;
  QPointer<PostprocessorWorkspace> workspace;
  std::unique_ptr<PostprocessorGuiObserver> observer;
};

#endif // POSTPROCESSORGUI_HPP
