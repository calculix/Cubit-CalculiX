#include "MaterialPropertiesWidget.hpp"
#include "GUI/properties/PropertyCommandText.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitMessage.hpp"
MaterialPropertiesWidget::MaterialPropertiesWidget(QWidget* parent) : PropertyTreeWidget(parent) {
}
bool MaterialPropertiesWidget::setMaterial(int id) {
  clear();
  currentEntityId = -1;
  FrameworkMaterialsInterface materials("calculix");
  const auto* material = materials.find(id);
  if (material == nullptr) {
    PRINT_ERROR("Cannot load material %d from the CalculiX workspace.\n", id);
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Material", id);
  addEditableName(general, id, QString::fromStdString(material->data.name));
  addValue(addGroup("definition", "Definition", PropertySection::Classification), "Workspace", "CalculiX");
  for (const auto& entry : material->data.cards) {
    const auto* definition = framework_materials().definitions().card(entry.first);
    if (definition == nullptr) {
      continue;
    }
    const auto* model = framework_materials().definitions().model(*definition, entry.second.model);
    if (model == nullptr) {
      continue;
    }
    auto* card = addGroup("material/" + QString::fromStdString(entry.first), QString::fromStdString(definition->title));
    QString state = "Inactive";
    if (entry.second.state == MaterialCardState::Active) {
      state = "Active";
    }
    addValue(card, "State", state);
    if (!model->title.empty()) {
      addValue(card, "Model", QString::fromStdString(model->title));
    }
    QStringList columns;
    for (const auto& column : model->columns) {
      QString label = QString::fromStdString(column.title);
      std::string unit;
      std::string error;
      if (column.quantity == "creep_coefficient") {
        std::string stressUnit;
        std::string timeUnit;
        if (framework_units().display_unit("calculix", "stress", stressUnit, error) && framework_units().display_unit("calculix", "time", timeUnit, error)) {
          label += " [(" + QString::fromStdString(stressUnit) + ")^(-n) " + QString::fromStdString(timeUnit) + "^(-(m+1))]";
        } else {
          PRINT_ERROR("%s\n", error.c_str());
        }
      } else if (framework_units().display_unit("calculix", column.quantity, unit, error)) {
        if (unit != "1") {
          label += " [" + QString::fromStdString(unit) + "]";
        }
      } else {
        PRINT_ERROR("%s\n", error.c_str());
      }
      columns.push_back(label);
    }
    if (model->scalar && entry.second.values.scalar.has_value()) {
      double value = 0.0;
      if (materials.get_material_property_value(id, model->property, value)) {
        addNumber(card, columns.front(), value);
      }
    } else if (!model->scalar && !entry.second.values.rows.empty()) {
      std::vector<std::vector<double>> rows;
      if (materials.get_material_property_value(id, model->property, rows)) {
        addTableRows(card, columns, rows);
      }
    }
    if (entry.second.values.referenceTemperature.has_value()) {
      double value = 0.0;
      if (materials.get_material_property_value(id, "CCX_EXPANSION_ZERO", value)) {
        std::string unit;
        std::string error;
        if (framework_units().display_unit("calculix", "temperature", unit, error)) {
          addNumber(card, "Reference temperature [" + QString::fromStdString(unit) + "]", value);
        }
      }
    }
  }
  return true;
}
void MaterialPropertiesWidget::applyEdit(const PropertyEditRequest& request) {
  QString name;
  if (!prepareNameEdit(request, currentEntityId, name)) {
    return;
  }
  FrameworkMaterialsInterface materials("calculix");
  const auto* material = materials.find(request.entityId);
  if (material == nullptr) {
    PRINT_ERROR("The selected material no longer exists.\n");
    return;
  }
  const QString command = QString("ccx modify material %1 name %2").arg(request.entityId).arg(name);
  submitNameCommand(request, QString::fromStdString(material->data.name), command, [this, id = request.entityId]() {
    return setMaterial(id);
  });
}
