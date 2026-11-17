#include "MaterialCardPresentation.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterials.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Core/units/UnitLabelFormat.hpp"
#include "CubitMessage.hpp"
const MaterialModelDefinition* MaterialCardPresentation::find(const std::string& property) {
  const FrameworkMaterialCardDefinition* card = nullptr;
  return framework_materials().definitions().property(property, card);
}
QStringList MaterialCardPresentation::columns(const std::string& property, const std::string& workspace, QStringList* unitTooltips)
{
  QStringList labels;
  if (unitTooltips != nullptr)
  {
    unitTooltips->clear();
  }
  const auto* model = find(property);
  if (model == nullptr)
  {
    return labels;
  }
  for (const auto& column : model->columns)
  {
    QString label = QString::fromStdString(column.title);
    QString rawUnit;
    std::string unit;
    std::string error;
    if (column.quantity == "creep_coefficient")
    {
      std::string stressUnit;
      std::string timeUnit;
      if (framework_units().display_unit(workspace, "stress", stressUnit, error) && framework_units().display_unit(workspace, "time", timeUnit, error))
      {
        rawUnit = "(" + QString::fromStdString(stressUnit) + ")^(-n)*(" + QString::fromStdString(timeUnit) + ")^(-(m+1))";
        label += " [" + UnitLabelFormat::power(stressUnit, QStringLiteral("-n")) + QStringLiteral("\u00b7") + UnitLabelFormat::power(timeUnit, QStringLiteral("-(m+1)")) + "]";
      }
      else
      {
        PRINT_ERROR("%s\n", error.c_str());
      }
    }
    else if (framework_units().display_unit(workspace, column.quantity, unit, error))
    {
      rawUnit = QString::fromStdString(unit);
      if (unit != "1")
      {
        label += " [" + UnitLabelFormat::format(unit) + "]";
      }
    }
    else
    {
      PRINT_ERROR("%s\n", error.c_str());
    }
    if (column.optional)
    {
      label += " (optional)";
    }
    labels.push_back(label);
    if (unitTooltips != nullptr)
    {
      unitTooltips->push_back("Input unit: " + rawUnit);
    }
  }
  return labels;
}
bool MaterialCardPresentation::enabled(const MaterialModelDefinition& definition, const MaterialData& material) {
  const FrameworkMaterialCardDefinition* card = nullptr;
  framework_materials().definitions().property(definition.property, card);
  if (card == nullptr) {
    return false;
  }
  auto found = material.cards.find(card->key);
  return found != material.cards.end() && found->second.state == MaterialCardState::Active && found->second.model == definition.key;
}
