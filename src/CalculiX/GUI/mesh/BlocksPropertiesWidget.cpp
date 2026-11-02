#include "BlocksPropertiesWidget.hpp"
#include "GUI/properties/PropertyCommandText.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitMessage.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include <QPointer>
#include <QStringList>

namespace
{
QStringList elementChoices(const std::vector<std::string>& values)
{
  QStringList choices;
  for (const auto& value : values)
  {
    choices.append(QString::fromStdString(value).toUpper());
  }
  choices.removeDuplicates();
  return choices;
}
}

BlocksPropertiesWidget::BlocksPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool BlocksPropertiesWidget::setBlock(int id)
{
  clear();
  currentBlockId = -1;
  CalculiXMeshInterface core;
  MeshProperties properties;
  std::string error;
  if (!core.get_block_properties(id, properties, error))
  {
    PRINT_ERROR("Cannot load block properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentBlockId = id;
  auto* general = addIdentity("Block", id);
  addEditableName(general, id, QString::fromStdString(properties.name));
  auto* mesh = addGroup("mesh", "Mesh", PropertySection::Data);
  addCommandChoice(mesh, id, "cubit_element_type", tr("Cubit element type"), QString::fromStdString(properties.elementType), elementChoices(core.get_valid_block_element_types(id)));
  addCommandChoice(mesh, id, "calculix_element_type", tr("CalculiX element type"), QString::fromStdString(properties.calculixElementType), elementChoices(core.get_compatible_ccx_element_types(id)));
  addValue(mesh, "Nodes", QString::number(static_cast<qulonglong>(properties.nodeCount)));
  addValue(mesh, "Elements", QString::number(static_cast<qulonglong>(properties.elementCount)));
  return true;
}

void BlocksPropertiesWidget::reload()
{
  if (currentBlockId < 1 || !isVisible())
  {
    return;
  }
  const int id = currentBlockId;
  const auto state = captureViewState();
  if (setBlock(id))
  {
    restoreViewState(state, true);
  }
}

void BlocksPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  if (!isVisible() || request.entityId != currentBlockId || request.revision != propertyEditRevision())
  {
    return;
  }
  CalculiXMeshInterface core;
  MeshProperties properties;
  std::string error;
  if (!core.get_block_properties(currentBlockId, properties, error))
  {
    PRINT_ERROR("Cannot edit block %d: %s\n", currentBlockId, error.c_str());
    reload();
    return;
  }
  QString command;
  QString original;
  if (request.field == "name")
  {
    original = QString::fromStdString(properties.name);
    QString quoted;
    QString quoteError;
    if (!PropertyCommandText::quoteName(request.value.trimmed(), quoted, quoteError))
    {
      PRINT_ERROR("Cannot rename block %d: %s\n", currentBlockId, quoteError.toStdString().c_str());
      return;
    }
    command = QString("block %1 name %2").arg(currentBlockId).arg(quoted);
  }
  else if (request.field == "cubit_element_type" || request.field == "calculix_element_type")
  {
    const bool native = request.field == "cubit_element_type";
    original = QString::fromStdString(native ? properties.elementType : properties.calculixElementType);
    const auto choices = elementChoices(native ? core.get_valid_block_element_types(currentBlockId) : core.get_compatible_ccx_element_types(currentBlockId));
    if (!choices.contains(request.value, Qt::CaseInsensitive))
    {
      PRINT_ERROR("The selected element type is no longer available for block %d.\n", currentBlockId);
      reload();
      return;
    }
    command = native ? QString("block %1 element type %2").arg(currentBlockId).arg(request.value.toUpper()) : QString("ccx block %1 element_type %2").arg(currentBlockId).arg(request.value.toUpper());
  }
  else
  {
    return;
  }
  if (original != request.original)
  {
    PRINT_WARNING("Block %d changed while its property editor was open. The property view has been refreshed.\n", currentBlockId);
    reload();
    return;
  }
  const QPointer<BlocksPropertiesWidget> self(this);
  const int id = currentBlockId;
  const bool submitted = GuiSendCmd::gui_cmd(command);
  if (self == nullptr)
  {
    return;
  }
  if (!submitted)
  {
    PRINT_ERROR("Could not submit the property command for block %d.\n", id);
  }
  // Submission is not command success. Display only values reloaded from the core.
  // Normally the existing command-completion observer has already refreshed this view.
  if (self->currentBlockId == id && self->propertyEditRevision() == request.revision)
  {
    self->reload();
  }
}
