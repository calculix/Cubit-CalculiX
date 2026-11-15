#include "CalculiXReportMaterials.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Reporting/ReportSection.hpp"
#include "Framework/Reporting/ReportTable.hpp"
#include <QString>
#include <utility>

bool CalculiXReportMaterials::collect(ReportSection& section, std::string& error) {
  FrameworkMaterialsInterface materials("calculix", MaterialValueSpace::Canonical);
  const auto& definitions = framework_materials().definitions();
  for (int id : materials.get_material_ids()) {
    const auto* material = materials.find(id);
    if (!material) {
      error = "A report material reference is unavailable.";
      return false;
    }
    for (const auto& entry : material->data.cards) {
      const auto& card = entry.second;
      if (card.state != MaterialCardState::Active) {
        continue;
      }
      const auto* definition = definitions.card(entry.first);
      if (!definition) {
        error = "A report material card has no Framework definition.";
        return false;
      }
      const auto* model = definitions.model(*definition, card.model);
      if (!model) {
        error = "A report material model has no Framework definition.";
        return false;
      }
      ReportTable table;
      table.title = material->data.name + " (" + std::to_string(id) + ") / " + definition->title + " / " + model->title;
      std::vector<SimUnits::QuantityUnit> quantities(model->columns.size());
      for (std::size_t column = 0; column < model->columns.size(); ++column) {
        const auto& property = model->columns[column];
        table.columns.push_back(property.title);
        if (property.quantity == "creep_coefficient") {
          table.units.push_back("Pa^(-n) s^(-m-1)");
          continue;
        }
        std::string unit;
        if (!framework_units().quantity(property.quantity, quantities[column], error) || !framework_units().display_unit("report", property.quantity, unit, error)) {
          return false;
        }
        table.units.push_back(unit);
      }
      std::vector<std::vector<double>> rows;
      if (model->scalar && card.values.scalar) {
        rows.push_back({*card.values.scalar});
      } else {
        for (const auto& row : card.values.rows) {
          rows.push_back(row.values);
        }
      }
      for (const auto& row : rows) {
        if (row.size() > model->columns.size()) {
          error = "A material report row exceeds its Framework schema.";
          return false;
        }
        std::vector<QJsonValue> values(model->columns.size(), QJsonValue(QString()));
        for (std::size_t column = 0; column < row.size(); ++column) {
          double value = row[column];
          if (model->columns[column].quantity != "creep_coefficient") {
            const auto& quantity = quantities[column];
            if (!framework_units().from_si(value, table.units[column], quantity.dimension, quantity.role, value, error)) {
              return false;
            }
          }
          values[column] = value;
        }
        table.rows.push_back(std::move(values));
      }
      section.tables.push_back(std::move(table));
      if (card.values.referenceTemperature.has_value()) {
        ReportTable reference;
        reference.title = material->data.name + " / Expansion reference temperature";
        reference.columns = {"Temperature"};
        SimUnits::QuantityUnit temperature;
        std::string unit;
        double value = *card.values.referenceTemperature;
        if (!framework_units().quantity("temperature", temperature, error) || !framework_units().display_unit("report", "temperature", unit, error) || !framework_units().from_si(value, unit, temperature.dimension, temperature.role, value, error)) {
          return false;
        }
        reference.units = {unit};
        reference.rows.push_back({value});
        section.tables.push_back(std::move(reference));
      }
    }
  }
  error.clear();
  return true;
}
