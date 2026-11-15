#include "ModelEditorInterface.hpp"
#include <utility>

namespace {
ModelEditorInterface::Open openEditor;
ModelEditorInterface::Close closeEditor;
}

void ModelEditorInterface::bind(Open open, Close close)
{
  openEditor = std::move(open);
  closeEditor = std::move(close);
}

void ModelEditorInterface::unbind() noexcept
{
  openEditor = {};
  closeEditor = {};
}

bool ModelEditorInterface::open(const std::string& workspace, const std::string& feature, int material, std::string& error)
{
  error.clear();
  if (!openEditor) {
    error = "The shared model editor requires an attached graphical workspace.";
    return false;
  }
  return openEditor(workspace, feature, material, error);
}

bool ModelEditorInterface::close(const std::string& workspace, const std::string& feature, std::string& error)
{
  error.clear();
  if (!closeEditor) {
    error = "The shared model editor is unavailable.";
    return false;
  }
  return closeEditor(workspace, feature, error);
}
