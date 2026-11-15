#pragma once
#include "CubitInterface.hpp"
#include <QPointer>
#include <QWidget>
#include <map>
#include <string>
class QMainWindow;
class QToolBar;
class QAction;
class ModelEditorManager : public CubitInterface::ObserverBase {
public:
  ModelEditorManager(QMainWindow* host, const std::string& componentDirectory);
  ~ModelEditorManager();
  void set_workspace(const std::string& workspace);
  void bind_toolbar(const std::string& workspace, QToolBar* toolbar);
  void clear();
  void notify_model_reset() override;
  void notify_cub_file_read(std::string filename) override;
  void notify_command_complete() override;
private:
  bool open(const std::string& workspace, const std::string& feature, int material, std::string& error);
  bool close(const std::string& workspace, const std::string& feature, std::string& error);
  bool submit(const std::string& command, std::string& error);
  QPointer<QMainWindow> host;
  QPointer<QToolBar> toolbar;
  std::map<std::string, QPointer<QToolBar>> workspaceToolbars;
  QPointer<QAction> separator;
  QPointer<QAction> materialsAction;
  QPointer<QAction> unitsAction;
  std::string activeWorkspace = "cubit";
  std::map<std::string, QPointer<QWidget>> editors;
};
