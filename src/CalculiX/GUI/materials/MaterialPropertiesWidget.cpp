#include "GUI/properties/PropertyCommandText.hpp"
#include "MaterialCardPresentation.hpp"
#include <map>
#include "MaterialPropertiesWidget.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "CubitMessage.hpp"

MaterialPropertiesWidget::MaterialPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool MaterialPropertiesWidget::setMaterial(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXMaterialsInterface core;
  MaterialProperties properties;
  std::string error;
  if (!core.get_material_properties(id, properties, error))
  {
    PRINT_ERROR("Cannot load material properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Material", id);
  addEditableName(general, id, QString::fromStdString(properties.name));
  addValue(addGroup("definition", "Definition", PropertySection::Classification), "Property group", QString::fromStdString(properties.group));
  std::map<QString, QTreeWidgetItem*> cards;
  for (const auto& property : properties.properties)
  {
    const QString rawName = QString::fromStdString(property.name);
    if (rawName.endsWith("_USE_CARD"))
    {
      continue;
    }
    const auto* definition = MaterialCardPresentation::find(property.name);
    if (definition != nullptr && !MaterialCardPresentation::enabled(*definition, properties))
    {
      continue;
    }
    QString title = definition == nullptr ? rawName : definition->title;
    if (definition == nullptr)
    {
      if (title.startsWith("CCX_"))
      {
        title.remove(0, 4);
      }
      title.replace('_', ' ');
    }
    auto*& card = cards[title];
    if (card == nullptr)
    {
      const QString key = definition == nullptr ? rawName : QString::fromStdString(definition->parentSwitch);
      card = addGroup("material/" + key, title);
    }
    if (definition != nullptr && !definition->model.isEmpty())
    {
      addValue(card, "Model", definition->model);
    }
    const auto columns = definition == nullptr ? QStringList{} : definition->columns;
    if (property.values.size() == 1 && property.values.front().size() == 1)
    {
      addNumber(card, columns.isEmpty() ? tr("Value") : columns.front(), property.values.front().front());
    }
    else
    {
      addTableRows(card, columns, property.values);
    }
  }

  return true;
}

void MaterialPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXMaterialsInterface core;
  MaterialProperties properties;
  std::string error;
  if (!core.get_material_properties(request.entityId, properties, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  QString originalName;
  QString quoteError;
  if (!PropertyCommandText::quoteName(QString::fromStdString(properties.name), originalName, quoteError))
  {
    PRINT_ERROR("Cannot quote the current material name: %s\n", quoteError.toStdString().c_str());
    return;
  }
  const QString command = QString("modify material %1 name %2").arg(originalName).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(properties.name), command, [this, id = request.entityId]() { return setMaterial(id); });
}
