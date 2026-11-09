#ifndef WORKSPACEMANAGER_HPP
#define WORKSPACEMANAGER_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"

#include <QHash>
#include <QByteArray>
#include <QObject>
#include <QPointer>
#include <memory>
#include <vector>

class Claro;
class DrawingInterface;
class CubitWorkspaceHost;
class WorkspaceHeader;
class QWidget;

class WorkspaceManager : public QObject
{
public:
  explicit WorkspaceManager(DrawingInterface& drawings);
  ~WorkspaceManager() override;
  bool attach(QWidget* host, std::string& error);
  bool register_views(const std::vector<WorkspaceProviderInterface*>& providers, std::string& error);
  bool activate(const QString& id, std::string& error);
  void shutdown() noexcept;

protected:
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  DrawingInterface& drawings;
  std::unique_ptr<CubitWorkspaceHost> host;
  QPointer<WorkspaceHeader> header;
  QHash<QString, WorkspaceView> views;
  QHash<QString, QByteArray> layouts;
  QHash<QString, unsigned int> bottomDockRevisions;
  QHash<QString, QPersistentModelIndex> selections;
  QString current = "cubit";
  bool ready = false;
  bool switching = false;
  bool stopped = false;
};

#endif // WORKSPACEMANAGER_HPP
