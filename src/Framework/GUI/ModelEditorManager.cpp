#include "ModelEditorManager.hpp"
#include "Resources/FrameworkIconLoader.hpp"
#include "materials/MaterialManagement.hpp"
#include "units/UnitsManagement.hpp"
#include "Framework/Interfaces/ModelEditorInterface.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "CommandDispatcher.hpp"
#include "Claro.hpp"
#include "CubitMessage.hpp"
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <exception>

ModelEditorManager::ModelEditorManager(QMainWindow* window, const std::string& componentDirectory) : host(window) {
  materialsAction = new QAction(FrameworkIconLoader::load(componentDirectory, "Materials.svg"), "Materials", window);
  materialsAction->setObjectName("CubitSim.Materials");
  materialsAction->setToolTip("Open workspace material management");
  unitsAction = new QAction(FrameworkIconLoader::load(componentDirectory, "Units.svg"), "Units", window);
  unitsAction->setObjectName("CubitSim.Units");
  unitsAction->setToolTip("Open workspace units");
  separator = new QAction(window);
  separator->setSeparator(true);
  QObject::connect(unitsAction.data(), &QAction::triggered, unitsAction.data(), [this]() {
    std::string prefix = "ccx";
    if (activeWorkspace == "postprocessor") {
      prefix = "post";
    } else if (activeWorkspace == "report") {
      prefix = "report";
    }
    std::string error;
    if (!submit(prefix + " units open", error)) {
      PRINT_ERROR("Cannot open workspace units: %s\n", error.c_str());
    }
  });
  QObject::connect(materialsAction.data(), &QAction::triggered, materialsAction.data(), [this]() {
    std::string error;
    if (!submit("ccx open materialmanagement", error)) {
      PRINT_ERROR("Cannot open workspace materials: %s\n", error.c_str());
    }
  });
  ModelEditorInterface::bind([this](const std::string& workspace, const std::string& feature, int material, std::string& error) { return open(workspace, feature, material, error); }, [this](const std::string& workspace, const std::string& feature, std::string& error) { return close(workspace, feature, error); });
  set_workspace("cubit");
  register_observer();
}
ModelEditorManager::~ModelEditorManager() {
  unregister_observer();
  ModelEditorInterface::unbind();
  clear();
  delete materialsAction.data();
  delete unitsAction.data();
  delete separator.data();
}
void ModelEditorManager::clear() {
  for (auto& editor : editors) {
    delete editor.second.data();
  }
  editors.clear();
}
void ModelEditorManager::bind_toolbar(const std::string& workspace, QToolBar* target) {
  workspaceToolbars[workspace] = target;
  if (activeWorkspace == workspace) {
    set_workspace(workspace);
  }
}
void ModelEditorManager::set_workspace(const std::string& workspace) {
  activeWorkspace = workspace;
  if (toolbar != nullptr) {
    toolbar->removeAction(materialsAction.data());
    toolbar->removeAction(unitsAction.data());
    toolbar->removeAction(separator.data());
  }
  toolbar = nullptr;
  const auto found = workspaceToolbars.find(workspace);
  if (found != workspaceToolbars.end()) {
    toolbar = found->second;
  }
  materialsAction->setEnabled(workspace == "calculix");
  materialsAction->setToolTip("Materials are authored in solver workspaces");
  if (workspace == "calculix") {
    materialsAction->setToolTip("Open CalculiX material management");
  }
  unitsAction->setEnabled(workspace == "calculix" || workspace == "postprocessor" || workspace == "report");
  if (toolbar != nullptr) {
    toolbar->addAction(separator.data());
    if (workspace == "calculix") {
      toolbar->addAction(materialsAction.data());
    }
    toolbar->addAction(unitsAction.data());
  }
  for (auto& editor : editors) {
    if (editor.second != nullptr) {
      editor.second->hide();
    }
  }
}
bool ModelEditorManager::submit(const std::string& command, std::string& error) {
  return CommandDispatcher::submit(qobject_cast<Claro*>(host.data()), QString::fromStdString(command), error);
}
bool ModelEditorManager::open(const std::string& workspace, const std::string& feature, int material, std::string& error) {
  if (host == nullptr) {
    error = "No GUI host is available.";
    return false;
  }
  std::string prefix;
  if (workspace == "calculix") {
    prefix = "ccx";
  } else if (workspace == "postprocessor") {
    prefix = "post";
  } else if (workspace == "report") {
    prefix = "report";
  } else {
    error = "The workspace has no material or unit schema.";
    return false;
  }
  if (feature != "units" && (feature != "materials" || workspace != "calculix")) {
    error = "This workspace does not author solver materials.";
    return false;
  }
  if (feature == "materials" && material > 0 && FrameworkMaterialsInterface(workspace).find(material) == nullptr) {
    error = "The selected material does not exist in this workspace.";
    return false;
  }
  const std::string key = workspace + "/" + feature;
  auto& editor = editors[key];
  if (editor == nullptr) {
    auto send = [this](const std::string& command, std::string& message) { return submit(command, message); };
    if (feature == "materials") {
      editor = new MaterialManagement(workspace, prefix, send, host);
    } else {
      editor = new UnitsManagement(workspace, prefix, send, host);
    }
    editor->setWindowFlag(Qt::Window, true);
  }
  if (feature == "materials") {
    auto* management = static_cast<MaterialManagement*>(editor.data());
    management->update();
    if (material > 0) {
      management->setMaterial(QString::number(material));
    }
  } else {
    static_cast<UnitsManagement*>(editor.data())->refresh();
  }
  editor->show();
  editor->raise();
  editor->activateWindow();
  return true;
}
bool ModelEditorManager::close(const std::string& workspace, const std::string& feature, std::string& error) {
  error.clear();
  auto found = editors.find(workspace + "/" + feature);
  if (found != editors.end() && found->second != nullptr) {
    bool closed = false;
    if (feature == "materials") {
      closed = static_cast<MaterialManagement*>(found->second.data())->close_from_command();
    } else if (feature == "units") {
      closed = static_cast<UnitsManagement*>(found->second.data())->close_from_command();
    }
    if (!closed) {
      error = "Closing the editor was cancelled.";
      return false;
    }
  }
  return true;
}

void ModelEditorManager::notify_model_reset() {
  clear();
}
void ModelEditorManager::notify_cub_file_read(std::string filename) {
  clear();
}
void ModelEditorManager::notify_command_complete() {
  try {
    for (auto& entry : editors) {
      if (entry.second != nullptr && entry.second->isVisible() && entry.first == "calculix/materials") {
        static_cast<MaterialManagement*>(entry.second.data())->update();
      }
    }
  } catch (const std::exception& exception) {
    PRINT_ERROR("Cannot refresh material editor after the command: %s\n", exception.what());
  } catch (...) {
    PRINT_ERROR("Cannot refresh material editor after the command.\n");
  }
}
