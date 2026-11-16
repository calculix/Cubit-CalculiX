#include "CalculiXReportMaterials.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Reporting/ReportSection.hpp"
#include "Framework/Reporting/ReportTable.hpp"
#include <QString>
#include <utility>

bool CalculiXReportMaterials::collect(ReportSection& section, std::string& error, ReportProgress* progress)
{
  std::size_t processedCards = 0;
  FrameworkMaterialsInterface materials("calculix", MaterialValueSpace::Canonical);
  // Read the solver's material store; convert its explicit data using Report preferences.
  FrameworkMaterialsInterface presentation("report", MaterialValueSpace::Display);
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
      if (!ReportProgress::advance(progress, ReportProgressStage::Model, "Collecting material tables", ++processedCards, 0, error))
      {
        return false;
      }
      ReportTable table;
      table.title = material->data.name + " (" + std::to_string(id) + ") / " + definition->title + " / " + model->title;
      std::vector<SimUnits::QuantityUnit> quantities(model->columns.size());
      table.column_quantities.resize(model->columns.size());
      std::string stressUnit, timeUnit;
      if (!framework_units().display_unit("report", "stress", stressUnit, error) ||
          !framework_units().display_unit("report", "time", timeUnit, error))
      {
        return false;
      }
      for (std::size_t column = 0; column < model->columns.size(); ++column) {
        const auto& property = model->columns[column];
        table.columns.push_back(property.title);
        if (property.quantity == "creep_coefficient") {
          table.columns.back() += " [(" + stressUnit + ")^(-n) * (" + timeUnit + ")^(-(m+1))]";
          table.units.push_back("");
          continue;
        }
        std::string unit;
        if (!framework_units().quantity(property.quantity, quantities[column], error) || !framework_units().display_unit("report", property.quantity, unit, error)) {
          return false;
        }
        table.units.push_back(unit);
        quantities[column].unit = unit;
        table.column_quantities[column] = quantities[column];
      }
      std::vector<std::vector<double>> rows;
      if (model->scalar)
      {
        if (card.values.scalar)
        {
          double value = 0.0;
          if (!presentation.read_scalar(material->data, model->property, value, error))
          {
            return false;
          }
          rows.push_back({value});
        }
      }
      else if (!presentation.read_table(material->data, model->property, rows, error))
      {
        return false;
      }
      for (const auto& row : rows) {
        if (row.size() > model->columns.size()) {
          error = "A material report row exceeds its Framework schema.";
          return false;
        }
        std::vector<QJsonValue> values(model->columns.size(), QJsonValue(QString()));
        std::vector<std::optional<ReportQuantity>> cellQuantities(model->columns.size());
        for (std::size_t column = 0; column < row.size(); ++column)
        {
          const double value = row[column];
          if (model->columns[column].quantity == "creep_coefficient")
          {
            if (row.size() < 3)
            {
              error = "Report creep coefficient requires n and m exponents.";
              return false;
            }
            SimUnits::QuantityUnit quantity;
            quantity.quantity = "creep_coefficient";
            // A has dimensions stress^(-n) * time^(-(m+1)).
            quantity.unit = "(" + stressUnit + ")^" + QString::number(-row[1], 'g', 17).toStdString() +
              "*(" + timeUnit + ")^" + QString::number(-(row[2] + 1.0), 'g', 17).toStdString();
            for (std::size_t axis = 0; axis < quantity.dimension.size(); ++axis)
            {
              quantity.dimension[axis] = -row[1] * SimUnits::Stress[axis] - (row[2] + 1.0) * SimUnits::Time[axis];
            }
            cellQuantities[column] = std::move(quantity);
          }
          values[column] = value;
        }
        table.rows.push_back(std::move(values));
        table.cell_quantities.push_back(std::move(cellQuantities));
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
        temperature.unit = unit;
        reference.column_quantities = {temperature};
        reference.rows.push_back({value});
        section.tables.push_back(std::move(reference));
      }
    }
  }
  error.clear();
  return true;
}
