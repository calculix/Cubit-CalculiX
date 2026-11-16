#include "ReportUnitValidation.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <functional>

ReportUnitValidation::ReportUnitValidation() = default;

bool ReportUnitValidation::configuration(const ReportConfiguration& value, std::string& error)
{
  if (!value.units)
  {
    return true;
  }
  SimUnits::WorkspaceUnitContext context;
  if (!framework_units().workspace_context("report", context, error))
  {
    return false;
  }
  value.units->apply_to(context);
  return framework_units().validate_workspace_context(context, error);
}

bool ReportUnitValidation::document(const ReportDocument& value, std::string& error)
{
  if (!value.validate(error) || !configuration(value.configuration, error))
  {
    return false;
  }
  if (value.source.source_units)
  {
    SimUnits::SourceUnitContext context;
    context.system = value.source.source_units->system;
    context.coordinate_unit = value.source.source_units->coordinate_unit;
    context.time_unit = value.source.source_units->time_unit;
    if (!framework_units().validate_source(context, error))
    {
      return false;
    }
  }
  const auto checkQuantity = [&](const ReportQuantity& quantity)
  {
    double ignored = 0.0;
    if (!framework_units().to_si(0.0, quantity.unit, quantity.dimension, quantity.role, ignored, error))
    {
      return false;
    }
    // These dimensions are derived per row by the solver/material provider.
    if (quantity.quantity == "creep_coefficient" || quantity.quantity == "constraint_coefficient")
    {
      return true;
    }
    SimUnits::QuantityUnit expected;
    if (!framework_units().quantity(quantity.quantity, expected, error))
    {
      return false;
    }
    const SimUnits::UnitDefinition supplied(quantity.unit, quantity.dimension, 1.0, quantity.role);
    const SimUnits::UnitDefinition required(expected.unit, expected.dimension, 1.0, expected.role);
    if (!SimUnits::CoreFrameworkUnits::compatible(supplied, required))
    {
      error = "Report quantity dimension or temperature role does not match: " + quantity.quantity;
      return false;
    }
    return true;
  };
  std::function<bool(const ReportSection&)> checkSection = [&](const ReportSection& section)
  {
    for (const auto& table : section.tables)
    {
      for (const auto& label : table.units)
      {
        if (!label.empty() && label != "unknown unit")
        {
          SimUnits::UnitDefinition definition;
          if (!framework_units().unit(label, definition, error))
          {
            return false;
          }
        }
      }
      for (const auto& quantity : table.column_quantities)
      {
        if (quantity && !checkQuantity(*quantity))
        {
          return false;
        }
      }
      for (const auto& row : table.cell_quantities)
      {
        for (const auto& quantity : row)
        {
          if (quantity && !checkQuantity(*quantity))
          {
            return false;
          }
        }
      }
    }
    for (const auto& child : section.children)
    {
      if (!checkSection(child))
      {
        return false;
      }
    }
    return true;
  };
  for (const auto& section : value.sections)
  {
    if (!checkSection(section))
    {
      return false;
    }
  }
  return true;
}
