#ifndef WORKSPACEGRAPHICSSTACK_HPP
#define WORKSPACEGRAPHICSSTACK_HPP

#include <QPointer>
#include <QList>
#include <string>

class QWidget;
class QLayout;
class QSplitter;
class QStackedWidget;

// Mounts pages without changing Claro::graphics_widget() or taking ownership of
// the native viewport. detach() restores it before module GUI teardown.
class WorkspaceGraphicsStack
{
public:
  ~WorkspaceGraphicsStack();
  bool attach(QWidget* nativeViewport, std::string& error);
  bool activate(QWidget* page, std::string& error);
  void detach() noexcept;

private:
  QPointer<QWidget> native;
  QPointer<QWidget> originalParent;
  QPointer<QLayout> originalLayout;
  QPointer<QSplitter> splitter;
  QPointer<QStackedWidget> stack;
  QList<QPointer<QWidget>> pages;
};

#endif // WORKSPACEGRAPHICSSTACK_HPP
