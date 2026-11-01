#ifndef WORKSPACEHEADER_HPP
#define WORKSPACEHEADER_HPP

#include <QMargins>
#include <QIcon>
#include <QPointer>
#include <QWidget>
#include <functional>

class QMainWindow;
class QTabBar;

class WorkspaceHeader : public QWidget
{
public:
  WorkspaceHeader(QMainWindow* host, std::function<void(const QString&)> request);
  ~WorkspaceHeader() override;
  void add_workspace(const QString& id, const QString& label, const QIcon& icon);
  void set_current(const QString& id);

protected:
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  void update_header();
  QPointer<QMainWindow> host;
  QMargins previousMargins;
  QTabBar* tabs = nullptr;
  QString current = "cubit";
  bool previousLayoutOnEntireRect = false;
  bool updatingHeader = false;
};

#endif // WORKSPACEHEADER_HPP
