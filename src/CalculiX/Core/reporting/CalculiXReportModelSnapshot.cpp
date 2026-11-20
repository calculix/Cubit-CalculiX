#include "CalculiX/Core/connections/CalculiXConnectionsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CalculiXReportDomainSettings.hpp"
#include "CalculiX/Core/units/CalculiXDomainUnits.hpp"
#include <QLocale>
#include <QRegularExpression>
#include "CalculiX/Core/units/CalculiXDofUnits.hpp"
#include <limits>
#include <cmath>
#include "CalculiXReportModelSnapshot.hpp"
#include "CalculiXReportMaterials.hpp"
#include "CalculiXReportImageSettings.hpp"
#include "CalculiXReportTemplateProvider.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include "Framework/Reporting/ReportRequest.hpp"
#include "CalculiX/Core/mesh/CalculiXMeshInterface.hpp"
#include "CalculiX/Core/sections/CalculiXSectionsInterface.hpp"
#include "CalculiX/Core/interactions/CalculiXInteractionsInterface.hpp"
#include "CalculiX/Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "CalculiX/Core/steps/CalculiXStepsInterface.hpp"
#include "CalculiX/Core/loads/CalculiXLoadsInterface.hpp"
#include "CalculiX/Core/bcs/CalculiXBCsInterface.hpp"
#include "CalculiX/Core/export/CalculiXExportInterface.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QStringList>
#include <set>
#include <map>
#include <utility>
#include <algorithm>

namespace
{
QJsonValue jsonValue(const std::string& value) {
  return QString::fromStdString(value);
}

QJsonValue jsonValue(double value) {
  return value;
}

QJsonValue jsonValue(int value) {
  return value;
}

template<class T>
QJsonValue jsonValue(const std::vector<T>& values) {
  QJsonArray result;
  for (const auto& value : values) {
    result.append(jsonValue(value));
  }
  return result;
}

template<class T>
void field(ReportTable& table, const char* label, const T& value)
{
  auto json = jsonValue(value);
  if (json.isArray())
  {
    const auto values = json.toArray();
    for (int i = 0; i < values.size(); ++i)
    {
      const auto name = std::string(label) + " / " + std::to_string(i + 1);
      auto value = values[i];
      if (value.isArray())
      {
        const auto entries = value.toArray();
        for (int entry = 0; entry < entries.size(); ++entry)
        {
          const auto detail = name + " / " + std::to_string(entry + 1);
          table.rows.push_back({QString::fromStdString(detail), entries[entry], ""});
          table.cell_quantities.push_back({std::nullopt, std::nullopt, std::nullopt});
        }
        continue;
      }
      table.rows.push_back({QString::fromStdString(name), value, ""});
      table.cell_quantities.push_back({std::nullopt, std::nullopt, std::nullopt});
    }
    return;
  }
  table.rows.push_back({label, json, ""});
  table.cell_quantities.push_back({std::nullopt, std::nullopt, std::nullopt});
}

// Domain settings already use Report units. Parse and annotate, never reconvert.
bool physicalField(ReportTable& table, const std::string& label, const QJsonValue& input,
  const std::string& quantityId, std::string& error)
{
  if (input.isArray())
  {
    const auto values = input.toArray();
    for (int i = 0; i < values.size(); ++i)
    {
      if (!physicalField(table, label + " / " + std::to_string(i + 1), values[i], quantityId, error))
      {
        return false;
      }
    }
    return true;
  }
  SimUnits::QuantityUnit quantity;
  if (!framework_units().quantity(quantityId, quantity, error) ||
      !framework_units().display_unit("report", quantityId, quantity.unit, error))
  {
    return false;
  }
  bool numeric = input.isDouble();
  double number = input.toDouble();
  const auto text = input.toString().trimmed();
  const auto components = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  if (input.isString() && components.size() > 1)
  {
    for (int index = 0; index < components.size(); ++index)
    {
      if (!physicalField(table, label + " / " + std::to_string(index + 1), components[index], quantityId, error))
      {
        return false;
      }
    }
    return true;
  }
  if (input.isString() && !text.isEmpty() && text.toLower() != "free" && text.toLower() != "fixed")
  {
    number = QLocale::c().toDouble(text, &numeric);
    if (!numeric)
    {
      error = "Invalid numeric report property: " + label + " / " + text.toStdString();
      return false;
    }
  }
  if (numeric)
  {
    if (!std::isfinite(number))
    {
      error = "Non-finite report property: " + label;
      return false;
    }
    table.rows.push_back({QString::fromStdString(label), number, QString::fromStdString(quantity.unit)});
    table.cell_quantities.push_back({std::nullopt, quantity, std::nullopt});
  }
  else
  {
    table.rows.push_back({QString::fromStdString(label), input, QString::fromStdString(quantity.unit)});
    table.cell_quantities.push_back({std::nullopt, std::nullopt, std::nullopt});
  }
  return true;
}

template<class T>
bool physicalRows(ReportTable& table, const char* label, const std::vector<std::vector<T>>& rows,
  const std::vector<std::string>& quantities, std::string& error)
{
  for (std::size_t row = 0; row < rows.size(); ++row)
  {
    for (std::size_t column = 0; column < rows[row].size(); ++column)
    {
      const auto name = std::string(label) + " " + std::to_string(row + 1) + " / " + std::to_string(column + 1);
      if (column < quantities.size() && !quantities[column].empty())
      {
        if (!physicalField(table, name, jsonValue(rows[row][column]), quantities[column], error))
        {
          return false;
        }
      }
      else
      {
        field(table, name.c_str(), rows[row][column]);
      }
    }
  }
  return true;
}

int rowId(const std::vector<std::string>& row) {
  bool ok = false;
  int id = -1;
  if (!row.empty()) {
    id = QString::fromStdString(row.front()).toInt(&ok);
  }
  if (ok && id > 0) {
    return id;
  }
  return -1;
}

ReportTable listing(const std::string& title, const std::vector<std::vector<std::string>>& rows) {
  ReportTable result;
  result.title = title;
  result.columns = {"ID", "Definition"};
  for (const auto& row : rows) {
    QStringList details;
    for (std::size_t i = 1; i < row.size(); ++i) {
      details.append(QString::fromStdString(row[i]));
    }
    result.rows.push_back({rowId(row), details.join(" | ")});
  }
  return result;
}
}

CalculiXReportModelSnapshot::CalculiXReportModelSnapshot() = default;

bool CalculiXReportModelSnapshot::collect(const ReportRequest& request, const ReportSource& source,
  ReportDocument& output, std::string& error) const
{
  const auto enabled = [&](const std::string& key)
  {
    const auto found = request.overrides.values.find(key);
    return found != request.overrides.values.end() && found->second == "true";
  };
  std::size_t processedTables = 0;
  CalculiXReportDomainSettings domainSettings;
  CalculiXMeshInterface mesh;
  CalculiXSectionsInterface sections;
  CalculiXInteractionsInterface interactions;
  CalculiXAmplitudesInterface amplitudes;
  CalculiXStepsInterface steps;
  CalculiXLoadsInterface loads;
  CalculiXBCsInterface bcs;
  CalculiXExportInterface custom;
  ReportTemplate definition;
  if (!CalculiXReportTemplateProvider().describe(definition, error)) {
    return false;
  }
  std::map<std::string, ReportSection> groups;
  for (const auto& item : definition.sections) {
    groups[item.id] = item;
  }
  auto& meshSection = groups["ccx.model.mesh"];
  ReportTable meshTable;
  meshTable.title = "Current mesh blocks";
  meshTable.columns = {"Block", "Name", "Cubit type", "CalculiX type", "Nodes", "Elements"};
  const auto blocks = mesh.get_blocks();
  std::size_t completedBlocks = 0;
  for (int id : blocks)
  {
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting mesh block " + std::to_string(id), completedBlocks++, blocks.size(), error))
    {
      return false;
    }
    MeshProperties properties;
    if (!mesh.get_block_properties(id, properties, error)) {
      return false;
    }
    meshTable.rows.push_back({id, QString::fromStdString(properties.name), QString::fromStdString(properties.elementType),
      QString::fromStdString(properties.calculixElementType), static_cast<double>(properties.nodeCount), static_cast<double>(properties.elementCount)});
  }
  meshSection.tables.push_back(meshTable);
  groups["ccx.model.blocks"].tables.push_back(std::move(meshTable));
  if (!CalculiXReportMaterials::collect(groups["ccx.model.materials"], error, request.progress)) {
    return false;
  }

  for (const auto& row : sections.get_sections_tree_data()) {
    SectionsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Sections / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Block", settings.block);
    field(table, "Material", settings.material);
    field(table, "Orientation", settings.orientation);
    if (!physicalField(table, "Thickness", jsonValue(settings.thickness), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Offset", jsonValue(settings.offset), "dimensionless", error))
    {
      return false;
    }
    field(table, "Beam type", settings.beamType);
    if (!physicalField(table, "Parameter 1", jsonValue(settings.parameter1), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Parameter 2", jsonValue(settings.parameter2), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Parameter 3", jsonValue(settings.parameter3), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Parameter 4", jsonValue(settings.parameter4), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Parameter 5", jsonValue(settings.parameter5), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Parameter 6", jsonValue(settings.parameter6), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "X", jsonValue(settings.x), "dimensionless", error))
    {
      return false;
    }
    if (!physicalField(table, "Y", jsonValue(settings.y), "dimensionless", error))
    {
      return false;
    }
    if (!physicalField(table, "Z", jsonValue(settings.z), "dimensionless", error))
    {
      return false;
    }
    if (!physicalField(table, "Offset 1", jsonValue(settings.offset1), "dimensionless", error))
    {
      return false;
    }
    if (!physicalField(table, "Offset 2", jsonValue(settings.offset2), "dimensionless", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.model.sections"].tables.push_back(std::move(table));
  }

  for (const auto& row : sections.get_orientations_tree_data()) {
    OrientationsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Orientations / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "System", settings.system);
    if (!physicalField(table, "A", jsonValue(settings.a), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "B", jsonValue(settings.b), "length", error))
    {
      return false;
    }
    field(table, "Axis", settings.axis);
    if (!physicalField(table, "Angle", jsonValue(settings.angle), "angle", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.model.sections"].tables.push_back(std::move(table));
  }

  CalculiXConnectionsInterface connections;
  for (const auto& body : connections.bodies())
  {
    ReportTable table;
    table.title = "Connections / Rigid body " + std::to_string(body.id);
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Name", body.name);
    field(table, "Attachment", body.geometry + " " + std::to_string(body.entity));
    field(table, "Reference point ID", body.point);
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }
  for (const auto& joint : connections.joints())
  {
    ReportTable table;
    table.title = "Connections / Joint " + std::to_string(joint.id);
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Name", joint.name);
    field(table, "Type", joint.type);
    field(table, "Endpoint A", joint.point_a);
    field(table, "Endpoint B", joint.point_b == 0 ? std::string("Ground") : std::to_string(joint.point_b));
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }
  for (const auto& row : interactions.get_constraints_tree_data()) {
    ConstraintsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Constraints / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    std::set<int> rotationNodes;
    if (!settings.terms.empty() && !CalculiXConnectionsInterface().get_rotation_node_ids(rotationNodes, error))
    {
      return false;
    }
    for (std::size_t index = 0; index < settings.terms.size(); ++index)
    {
      const auto& term = settings.terms[index];
      if (term.size() != 3 || !std::isfinite(term[0]) || term[0] <= 0.0 ||
          term[0] > static_cast<double>((std::numeric_limits<int>::max)()) || std::floor(term[0]) != term[0] ||
          !std::isfinite(term[1]) || term[1] < 1.0 || term[1] > 11.0 || std::floor(term[1]) != term[1])
      {
        error = "Invalid report constraint equation term.";
        return false;
      }
      const auto prefix = "Equation term " + std::to_string(index + 1);
      field(table, (prefix + " / node").c_str(), term[0]);
      field(table, (prefix + " / DOF").c_str(), term[1]);
      std::string quantityId, unit;
      if (!CalculiXDofUnits::equation_quantity(static_cast<int>(term[1]),
          rotationNodes.count(static_cast<int>(term[0])) != 0, quantityId, error) ||
          !framework_units().display_unit("report", quantityId, unit, error))
      {
        return false;
      }
      // Compound units require a difference symbol instead of an affine temperature.
      if (quantityId == "temperature_difference")
      {
        if (unit == "degC")
        {
          unit = "delta_degC";
        }
        else if (unit == "degF")
        {
          unit = "delta_degF";
        }
        else if (unit == "K")
        {
          unit = "delta_K";
        }
      }
      ReportQuantity coefficient;
      coefficient.quantity = "constraint_coefficient";
      coefficient.unit = "1/(" + unit + ")";
      SimUnits::UnitDefinition definition;
      if (!framework_units().unit(coefficient.unit, definition, error))
      {
        return false;
      }
      coefficient.dimension = definition.dimension;
      coefficient.role = definition.role;
      table.rows.push_back({QString::fromStdString(prefix + " / coefficient"), term[2], QString::fromStdString(coefficient.unit)});
      table.cell_quantities.push_back({std::nullopt, coefficient, std::nullopt});
    }
    field(table, "Equation pairs", settings.equationPairs);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Block", settings.block);
    field(table, "Nodeset", settings.nodeset);
    field(table, "Reference", settings.reference);
    field(table, "Rotation", settings.rotation);
    field(table, "Master", settings.master);
    field(table, "Slave", settings.slave);
    if (!physicalField(table, "Tolerance", jsonValue(settings.tolerance), "length", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }

  for (const auto& row : interactions.get_surfaceinteractions_tree_data()) {
    SurfaceInteractionsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Surface Interactions / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    if (!physicalRows(table, "Pressure overclosure", settings.pressureOverclosure, {"stress", "length"}, error))
    {
      return false;
    }
    if (!physicalRows(table, "Conductance", settings.conductance, {"heat_transfer", "stress", "temperature"}, error))
    {
      return false;
    }
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Behavior", settings.behavior);
    if (!physicalField(table, "C 0", jsonValue(settings.c0), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "P 0", jsonValue(settings.p0), "stress", error))
    {
      return false;
    }
    if (!physicalField(table, "Slope", jsonValue(settings.slope), "force_per_volume", error))
    {
      return false;
    }
    if (!physicalField(table, "Sigma", jsonValue(settings.sigma), "stress", error))
    {
      return false;
    }
    if (!physicalField(table, "Linear c 0", jsonValue(settings.linearC0), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Tied slope", jsonValue(settings.tiedSlope), "force_per_volume", error))
    {
      return false;
    }
    field(table, "Conversion", settings.conversion);
    field(table, "Weight", settings.weight);
    if (!physicalField(table, "Velocity", jsonValue(settings.velocity), "velocity", error))
    {
      return false;
    }
    field(table, "Mu", settings.mu);
    if (!physicalField(table, "Lambda value", jsonValue(settings.lambdaValue), "force_per_volume", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }

  for (const auto& row : interactions.get_contactpairs_tree_data()) {
    ContactPairsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Contact Pairs / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Surface interaction", settings.surfaceInteraction);
    field(table, "Contact type", settings.contactType);
    field(table, "Master", settings.master);
    field(table, "Slave", settings.slave);
    if (!physicalField(table, "Adjust", jsonValue(settings.adjust), "length", error))
    {
      return false;
    }
    field(table, "Adjust nodeset", settings.adjustNodeset);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }

  for (const auto& row : amplitudes.get_amplitudes_tree_data()) {
    AmplitudesSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Amplitudes / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    if (!physicalRows(table, "Points", settings.points, {settings.axis, "dimensionless"}, error))
    {
      return false;
    }
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Axis", settings.axis);
    if (!physicalField(table, "Shift x", jsonValue(settings.shiftX), settings.axis, error))
    {
      return false;
    }
    field(table, "Shift y", settings.shiftY);
    field(table, "Total time", settings.totalTime);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : bcs.get_bcsdisplacements_tree_data()) {
    BCsDisplacementsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "B Cs Displacements / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Nodeset", settings.nodeset);
    if (!physicalField(table, "Translation X", jsonValue(settings.component1), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Translation Y", jsonValue(settings.component2), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Translation Z", jsonValue(settings.component3), "length", error))
    {
      return false;
    }
    if (!physicalField(table, "Rotation X", jsonValue(settings.component4), "angle", error))
    {
      return false;
    }
    if (!physicalField(table, "Rotation Y", jsonValue(settings.component5), "angle", error))
    {
      return false;
    }
    if (!physicalField(table, "Rotation Z", jsonValue(settings.component6), "angle", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : bcs.get_bcstemperatures_tree_data()) {
    BCsTemperaturesSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "B Cs Temperatures / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Nodeset", settings.nodeset);
    if (!physicalField(table, "Magnitude", jsonValue(settings.magnitude), "temperature", error))
    {
      return false;
    }
    field(table, "Keyword", settings.keyword);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_initialconditions_tree_data()) {
    InitialConditionsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Initial Conditions / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    if (!physicalRows(table, "Block stress", settings.blockStress, {"", "stress", "stress", "stress", "stress", "stress", "stress"}, error))
    {
      return false;
    }
    if (!physicalRows(table, "Element stress", settings.elementStress, {"", "", "stress", "stress", "stress", "stress", "stress", "stress"}, error))
    {
      return false;
    }
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Bc id", settings.bcId);
    field(table, "Condition type", settings.conditionType);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.model.initial_conditions"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_fieldoutputs_tree_data()) {
    FieldOutputsSettings settings;
    if (!steps.get_fieldoutput_settings(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Field Outputs / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Parameters", settings.parameters);
    field(table, "Enabled keys", settings.enabledKeys);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.steps"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_historyoutputs_tree_data()) {
    HistoryOutputsSettings settings;
    if (!steps.get_historyoutput_settings(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "History Outputs / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Parameters", settings.parameters);
    field(table, "Enabled keys", settings.enabledKeys);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.steps"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsforces_tree_data()) {
    LoadsForcesSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Forces / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, settings.referencePoint ? "Reference point" : "Nodeset", settings.referencePoint ? std::to_string(settings.referencePoint) : settings.nodeset);
    if (!physicalField(table, "Force X", jsonValue(settings.component1), "force", error))
    {
      return false;
    }
    if (!physicalField(table, "Force Y", jsonValue(settings.component2), "force", error))
    {
      return false;
    }
    if (!physicalField(table, "Force Z", jsonValue(settings.component3), "force", error))
    {
      return false;
    }
    if (!physicalField(table, "Moment X", jsonValue(settings.component4), "moment", error))
    {
      return false;
    }
    if (!physicalField(table, "Moment Y", jsonValue(settings.component5), "moment", error))
    {
      return false;
    }
    if (!physicalField(table, "Moment Z", jsonValue(settings.component6), "moment", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadspressures_tree_data()) {
    LoadsPressuresSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Pressures / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    if (!physicalField(table, "Magnitude", jsonValue(settings.magnitude), "pressure", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsheatfluxes_tree_data()) {
    LoadsHeatfluxesSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Heatfluxes / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    if (!physicalField(table, "Magnitude", jsonValue(settings.magnitude), "heat_flux", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsgravity_tree_data()) {
    LoadsGravitySettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Gravity / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Block", settings.block);
    if (!physicalField(table, "Magnitude", jsonValue(settings.magnitude), "acceleration", error))
    {
      return false;
    }
    field(table, "Direction", settings.direction);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadscentrifugal_tree_data()) {
    LoadsCentrifugalSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Centrifugal / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Block", settings.block);
    if (!physicalField(table, "Magnitude", jsonValue(settings.magnitude), "frequency_squared", error))
    {
      return false;
    }
    if (!physicalField(table, "Coordinate", jsonValue(settings.coordinate), "length", error))
    {
      return false;
    }
    field(table, "Direction", settings.direction);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadstrajectory_tree_data()) {
    LoadsTrajectorySettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Trajectory / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    std::vector<std::string> sampleQuantities = {"length", "heat_flux"};
    if (settings.type == 2)
    {
      sampleQuantities = {"length", "length", "body_heat_flux"};
    }
    if (!physicalRows(table, "Samples", settings.samples, sampleQuantities, error))
    {
      return false;
    }
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Load type", settings.loadType);
    field(table, "Curve", settings.curve);
    field(table, "Vertex", settings.vertex);
    field(table, "Surfaces", settings.surfaces);
    field(table, "Direction", settings.direction);
    if (!physicalField(table, "Time begin", jsonValue(settings.timeBegin), "time", error))
    {
      return false;
    }
    if (!physicalField(table, "Time end", jsonValue(settings.timeEnd), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Model change", settings.modelChange);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsfilm_tree_data()) {
    LoadsFilmSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Film / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    if (!physicalField(table, "Temperature", jsonValue(settings.temperature), "temperature", error))
    {
      return false;
    }
    if (!physicalField(table, "Coefficient", jsonValue(settings.coefficient), "heat_transfer", error))
    {
      return false;
    }
    field(table, "Secondary amplitude", settings.secondaryAmplitude);
    if (!physicalField(table, "Secondary time delay", jsonValue(settings.secondaryTimeDelay), "time", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsradiation_tree_data()) {
    LoadsRadiationSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Radiation / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    if (!physicalField(table, "Temperature", jsonValue(settings.temperature), "temperature", error))
    {
      return false;
    }
    field(table, "Coefficient", settings.coefficient);
    field(table, "Secondary amplitude", settings.secondaryAmplitude);
    if (!physicalField(table, "Secondary time delay", jsonValue(settings.secondaryTimeDelay), "time", error))
    {
      return false;
    }
    field(table, "Cavity", settings.cavity);
    field(table, "Cavity radiation", settings.cavityRadiation);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadssurfacetraction_tree_data()) {
    LoadsSurfaceTractionSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Surface Traction / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    if (!physicalField(table, "Force X", jsonValue(settings.component1), "force", error))
    {
      return false;
    }
    if (!physicalField(table, "Force Y", jsonValue(settings.component2), "force", error))
    {
      return false;
    }
    if (!physicalField(table, "Force Z", jsonValue(settings.component3), "force", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsbodyheatflux_tree_data()) {
    LoadsBodyHeatfluxSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Loads Body Heatflux / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    if (!physicalField(table, "Time delay", jsonValue(settings.timeDelay), "time", error))
    {
      return false;
    }
    field(table, "Operation", settings.operation);
    if (!physicalField(table, "Magnitude", jsonValue(settings.magnitude), "body_heat_flux", error))
    {
      return false;
    }
    field(table, "Entity type", settings.entityType);
    field(table, "Ids", settings.ids);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_physicalconstants_tree_data()) {
    PhysicalConstantsSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Physical constants";
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    if (!physicalField(table, "Absolute zero", jsonValue(settings.absoluteZero), "temperature", error))
    {
      return false;
    }
    if (!physicalField(table, "Stefan boltzmann", jsonValue(settings.stefanBoltzmann), "stefan_boltzmann", error))
    {
      return false;
    }
    if (!physicalField(table, "Newton gravity", jsonValue(settings.newtonGravity), "newton_gravity", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_damping_tree_data()) {
    DampingSettings settings;
    if (!domainSettings.get(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Damping";
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    if (!physicalField(table, "Alpha", jsonValue(settings.alpha), "inverse_time", error))
    {
      return false;
    }
    if (!physicalField(table, "Beta", jsonValue(settings.beta), "time", error))
    {
      return false;
    }
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : custom.get_customlines_tree_data()) {
    CustomLinesSettings settings;
    if (!custom.get_customline_settings(rowId(row), settings, error)) {
      return false;
    }
    ReportTable table;
    table.title = "Custom input lines";
    table.columns = {"Property", "Value", "Unit"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Position", settings.position);
    field(table, "Keyword", settings.keyword);
    field(table, "Keyword id", settings.keywordId);
    field(table, "Text", settings.text);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  if (enabled("ccx.model.sets"))
  {
    ReportTable summary;
    summary.title = "Sets and surfaces";
    summary.columns = {"Kind", "ID", "Name", "Members"};
    for (const auto& row : mesh.get_nodeset_tree_data())
    {
      const int id = rowId(row);
      MeshProperties properties;
      if (!mesh.get_nodeset_properties(id, properties, error))
      {
        return false;
      }
      summary.rows.push_back({"Node set", id, QString::fromStdString(properties.name), static_cast<double>(properties.nodeCount)});
      if (enabled("ccx.model.sets.nodes"))
      {
        ReportTable members;
        members.title = "Node set " + std::to_string(id) + " / membership";
        members.columns = {"Node"};
        for (int node : mesh.get_nodeset_node_ids(id))
        {
          members.rows.push_back({node});
        }
        groups["ccx.model.sets"].tables.push_back(std::move(members));
      }
    }
    for (const auto& row : mesh.get_sideset_tree_data())
    {
      const int id = rowId(row);
      MeshProperties properties;
      if (!mesh.get_sideset_properties(id, properties, error))
      {
        return false;
      }
      summary.rows.push_back({"Side set", id, QString::fromStdString(properties.name), static_cast<double>(properties.sideCount)});
      if (enabled("ccx.model.sets.faces") || enabled("ccx.model.sets.elements"))
      {
        std::vector<std::pair<int, int>> sides;
        if (!mesh.get_sideset_members(id, sides, error))
        {
          return false;
        }
        ReportTable members;
        members.title = "Side set " + std::to_string(id) + " / membership (native Cubit sides)";
        members.columns = {"Element", "Native side"};
        std::set<int> elements;
        for (const auto& side : sides)
        {
          if (enabled("ccx.model.sets.faces"))
          {
            members.rows.push_back({side.first, side.second});
          }
          else if (elements.insert(side.first).second)
          {
            members.rows.push_back({side.first, ""});
          }
        }
        groups["ccx.model.sets"].tables.push_back(std::move(members));
      }
    }
    groups["ccx.model.sets"].tables.insert(groups["ccx.model.sets"].tables.begin(), std::move(summary));
  }
  if (enabled("ccx.model.mesh") && enabled("ccx.model.mesh.nodes"))
  {
    std::set<int> ids;
    for (int block : blocks)
    {
      const auto nodes = mesh.get_block_node_ids(block);
      ids.insert(nodes.begin(), nodes.end());
    }
    const std::vector<int> ordered(ids.begin(), ids.end());
    ReportTable nodes;
    nodes.title = "Node coordinates";
    nodes.columns = {"Node", "X", "Y", "Z"};
    std::string unit;
    if (!framework_units().display_unit("report", "length", unit, error))
    {
      return false;
    }
    nodes.units = {"", unit, unit, unit};
    const SimUnits::QuantityUnit coordinates("length", unit, SimUnits::Length);
    nodes.column_quantities = {std::nullopt, ReportQuantity(coordinates), ReportQuantity(coordinates), ReportQuantity(coordinates)};
    for (const auto& row : mesh.get_node_coordinates(ordered))
    {
      std::vector<QJsonValue> values = {row[0]};
      for (std::size_t axis = 1; axis < 4; ++axis)
      {
        double si = 0.0, converted = 0.0;
        if (!framework_units().geometry_to_si(row[axis], si, error) ||
            !framework_units().from_si(si, unit, SimUnits::Length, SimUnits::UnitRole::Linear, converted, error))
        {
          return false;
        }
        values.push_back(converted);
      }
      nodes.rows.push_back(std::move(values));
    }
    meshSection.tables.push_back(std::move(nodes));
  }
  if (enabled("ccx.model.mesh") && enabled("ccx.model.mesh.elements"))
  {
    ReportTable elements;
    elements.title = "Element connectivity";
    elements.columns = {"Element", "Native type", "Connectivity"};
    for (const auto& row : mesh.get_element_id_type_connectivity())
    {
      if (row.size() < 2)
      {
        continue;
      }
      QStringList nodes;
      for (std::size_t index = 2; index < row.size(); ++index)
      {
        nodes.push_back(QString::number(row[index]));
      }
      elements.rows.push_back({row[0], row[1], nodes.join(", ")});
    }
    meshSection.tables.push_back(std::move(elements));
  }
  int stepOrdinal = 0;
  for (int id : steps.get_steps_ids()) {
    ++stepOrdinal;
    StepsSettings settings;
    if (!domainSettings.get(id, settings, error)) {
      return false;
    }
    ReportSection step;
    step.id = "ccx.steps." + std::to_string(id);
    step.title = "Analysis step " + std::to_string(stepOrdinal) + " (ID " + std::to_string(id) + "): " + settings.name;
    ReportTable parameters;
    parameters.title = "Procedure and parameters";
    parameters.columns = {"Property", "Value", "Unit"};
    field(parameters, "type", settings.type);
    const auto quantities = CalculiXDomainUnits::procedure_quantities(settings.type, settings.procedure);
    for (std::size_t index = 0; index < settings.procedure.size(); ++index)
    {
      const auto label = "Procedure parameter " + std::to_string(index);
      if (!quantities[index].empty())
      {
        if (!physicalField(parameters, label + " (" + quantities[index] + ")", jsonValue(settings.procedure[index]), quantities[index], error))
        {
          return false;
        }
      }
      else
      {
        field(parameters, label.c_str(), settings.procedure[index]);
      }
    }
    field(parameters, "parameters", settings.parameters);
    if (!ReportProgress::advance(request.progress, ReportProgressStage::Model, "Collecting model tables and analysis steps", ++processedTables, 0, error))
    {
      return false;
    }
    step.tables.push_back(std::move(parameters));
    {
      ReportSection assigned;
      assigned.id = step.id + ".boundary_conditions";
      assigned.title = "Boundary conditions assignments";
      assigned.tables.push_back(listing("bcsdisplacements", steps.get_steps_bcsdisplacements_tree_data(id)));
      assigned.tables.push_back(listing("bcstemperatures", steps.get_steps_bcstemperatures_tree_data(id)));
      step.children.push_back(std::move(assigned));
    }
    {
      ReportSection assigned;
      assigned.id = step.id + ".loads";
      assigned.title = "Loads assignments";
      assigned.tables.push_back(listing("loadsforces", steps.get_steps_loadsforces_tree_data(id)));
      assigned.tables.push_back(listing("loadspressures", steps.get_steps_loadspressures_tree_data(id)));
      assigned.tables.push_back(listing("loadsheatfluxes", steps.get_steps_loadsheatfluxes_tree_data(id)));
      assigned.tables.push_back(listing("loadsgravity", steps.get_steps_loadsgravity_tree_data(id)));
      assigned.tables.push_back(listing("loadscentrifugal", steps.get_steps_loadscentrifugal_tree_data(id)));
      assigned.tables.push_back(listing("loadstrajectory", steps.get_steps_loadstrajectory_tree_data(id)));
      assigned.tables.push_back(listing("loadsfilm", steps.get_steps_loadsfilm_tree_data(id)));
      assigned.tables.push_back(listing("loadsradiation", steps.get_steps_loadsradiation_tree_data(id)));
      assigned.tables.push_back(listing("loadssurfacetraction", steps.get_steps_loadssurfacetraction_tree_data(id)));
      assigned.tables.push_back(listing("loadsbodyheatflux", steps.get_steps_loadsbodyheatflux_tree_data(id)));
      step.children.push_back(std::move(assigned));
    }
    {
      ReportSection assigned;
      assigned.id = step.id + ".field_output";
      assigned.title = "Field output assignments";
      assigned.tables.push_back(listing("fieldoutputs", steps.get_steps_fieldoutputs_tree_data(id)));
      step.children.push_back(std::move(assigned));
    }
    {
      ReportSection assigned;
      assigned.id = step.id + ".history_output";
      assigned.title = "History output assignments";
      assigned.tables.push_back(listing("historyoutputs", steps.get_steps_historyoutputs_tree_data(id)));
      step.children.push_back(std::move(assigned));
    }

    groups["ccx.steps"].children.push_back(std::move(step));
  }
  ReportTable modelUnits;
  modelUnits.title = "Model presentation units";
  modelUnits.columns = {"Quantity", "Unit"};
  for (const auto& quantity : framework_units().quantity_ids()) {
    std::string unit;
    if (!framework_units().display_unit("report", quantity, unit, error)) {
      return false;
    }
    modelUnits.rows.push_back({QString::fromStdString(quantity), QString::fromStdString(unit)});
  }
  for (auto& group : groups) {
    if (group.first.rfind("ccx.", 0) == 0 && (!group.second.tables.empty() || !group.second.children.empty())) {
      group.second.tables.push_back(modelUnits);
    }
  }
  groups["ccx.model.overview"].paragraphs.push_back("Current model captured from CalculiX Core. Physical model values use Report presentation units. Amplitude ordinates and normalized offsets are dimensionless. Custom solver text retains its literal representation. Historical jobs retain their original source units.");
  ReportDocument candidate = output;
  candidate.source = source;
  // Hash collected report content, including requested detail. This identifies
  // a reporting snapshot, not an unrecorded solver run or full model archive.
  QJsonArray captured;
  for (const auto& item : groups) {
    captured.append(item.second.to_json());
  }
  candidate.source.snapshot_fingerprint = QCryptographicHash::hash(QJsonDocument(captured).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex().toStdString();
  std::set<int> selected;
  const auto selection = request.overrides.values.find("steps");
  std::vector<int> selectedIds;
  std::string requestedSteps = "all";
  if (selection != request.overrides.values.end()) {
    requestedSteps = selection->second;
  }
  if (!CalculiXReportImageSettings::select_steps(steps.get_steps_ids(), requestedSteps, selectedIds, error)) {
    return false;
  }
  selected.insert(selectedIds.begin(), selectedIds.end());
  auto& stepSections = groups["ccx.steps"].children;
  stepSections.erase(std::remove_if(stepSections.begin(), stepSections.end(), [&](ReportSection& step) {
    const int id = QString::fromStdString(step.id).section('.', -1).toInt();
    step.children.erase(std::remove_if(step.children.begin(), step.children.end(), [&](const ReportSection& child) {
      const auto key = "ccx.steps." + QString::fromStdString(child.id).section('.', -1).toStdString();
      return !enabled(key);
    }), step.children.end());
    if (!enabled("ccx.steps.parameters")) {
      step.tables.clear();
    }
    return !selected.empty() && selected.count(id) == 0;
  }), stepSections.end());
  candidate.sections.clear();
  for (const auto& item : definition.sections) {
    if (item.id.rfind("post.", 0) != 0 && enabled(item.id)) {
      candidate.sections.push_back(std::move(groups[item.id]));
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
