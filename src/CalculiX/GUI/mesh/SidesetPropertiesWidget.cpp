#include "SidesetPropertiesWidget.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitMessage.hpp"

SidesetPropertiesWidget::SidesetPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool SidesetPropertiesWidget::setSideset(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXMeshInterface core;
  MeshProperties properties;
  std::string error;
  if (!core.get_sideset_properties(id, properties, error))
  {
    PRINT_ERROR("Cannot load sideset properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Sideset", id);
  addEditableName(general, id, QString::fromStdString(properties.name));
  auto* mesh = addGroup("mesh", "Mesh", PropertySection::Data);
  addValue(mesh, "Sides", QString::number(static_cast<qulonglong>(properties.sideCount)));
  return true;
}

void SidesetPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXMeshInterface core;
  MeshProperties properties;
  std::string error;
  if (!core.get_sideset_properties(request.entityId, properties, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("sideset %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(properties.name), command, [this, id = request.entityId]() { return setSideset(id); });
}
