#include "NodesetPropertiesWidget.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitMessage.hpp"

NodesetPropertiesWidget::NodesetPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool NodesetPropertiesWidget::setNodeset(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXMeshInterface core;
  MeshProperties properties;
  std::string error;
  if (!core.get_nodeset_properties(id, properties, error))
  {
    PRINT_ERROR("Cannot load nodeset properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Nodeset", id);
  addEditableName(general, id, QString::fromStdString(properties.name));
  auto* mesh = addGroup("mesh", "Mesh", PropertySection::Data);
  addValue(mesh, "Nodes", QString::number(static_cast<qulonglong>(properties.nodeCount)));
  return true;
}

void NodesetPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXMeshInterface core;
  MeshProperties properties;
  std::string error;
  if (!core.get_nodeset_properties(request.entityId, properties, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("nodeset %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(properties.name), command, [this, id = request.entityId]() { return setNodeset(id); });
}
