#include "CalculiXReportModelSnapshot.hpp"
#include "CalculiXReportImageSettings.hpp"
#include "CalculiXReportTemplateProvider.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include "Framework/Reporting/ReportRequest.hpp"
#include "CalculiX/Core/mesh/CalculiXMeshInterface.hpp"
#include "CalculiX/Core/materials/CalculiXMaterialsInterface.hpp"
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
QJsonValue jsonValue(const std::string& value)
{
  return QString::fromStdString(value);
}

QJsonValue jsonValue(double value)
{
  return value;
}

QJsonValue jsonValue(int value)
{
  return value;
}

template<class T>
QJsonValue jsonValue(const std::vector<T>& values)
{
  QJsonArray result;
  for (const auto& value : values)
  {
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
    json = QString::fromUtf8(QJsonDocument(json.toArray()).toJson(QJsonDocument::Compact));
  }
  table.rows.push_back({label, json});
}

int rowId(const std::vector<std::string>& row)
{
  bool ok = false;
  const int id = row.empty() ? -1 : QString::fromStdString(row.front()).toInt(&ok);
  return ok && id > 0 ? id : -1;
}

ReportTable listing(const std::string& title, const std::vector<std::vector<std::string>>& rows)
{
  ReportTable result;
  result.title = title;
  result.columns = {"ID", "Definition"};
  for (const auto& row : rows)
  {
    QStringList details;
    for (std::size_t i = 1; i < row.size(); ++i)
    {
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
  CalculiXMeshInterface mesh;
  CalculiXMaterialsInterface materials;
  CalculiXSectionsInterface sections;
  CalculiXInteractionsInterface interactions;
  CalculiXAmplitudesInterface amplitudes;
  CalculiXStepsInterface steps;
  CalculiXLoadsInterface loads;
  CalculiXBCsInterface bcs;
  CalculiXExportInterface custom;
  ReportTemplate definition;
  if (!CalculiXReportTemplateProvider().describe(definition, error))
  {
    return false;
  }
  std::map<std::string, ReportSection> groups;
  for (const auto& item : definition.sections)
  {
    groups[item.id] = item;
  }
  auto& meshSection = groups["ccx.model.mesh"];
  ReportTable meshTable;
  meshTable.title = "Current mesh blocks";
  meshTable.columns = {"Block", "Name", "Cubit type", "CalculiX type", "Nodes", "Elements"};
  for (int id : mesh.get_blocks())
  {
    MeshProperties properties;
    if (!mesh.get_block_properties(id, properties, error))
    {
      return false;
    }
    meshTable.rows.push_back({id, QString::fromStdString(properties.name), QString::fromStdString(properties.elementType),
      QString::fromStdString(properties.calculixElementType), static_cast<double>(properties.nodeCount), static_cast<double>(properties.elementCount)});
  }
  meshSection.tables.push_back(meshTable);
  groups["ccx.model.blocks"].tables.push_back(std::move(meshTable));
  for (const auto& row : materials.get_material_tree_data())
  {
    MaterialProperties properties;
    if (!materials.get_material_properties(rowId(row), properties, error))
    {
      return false;
    }
    for (const auto& property : properties.properties)
    {
      ReportTable table;
      table.title = properties.name + " / " + property.name;
      std::size_t width = 1;
      for (const auto& values : property.values)
      {
        width = std::max(width, values.size());
      }
      for (std::size_t i = 0; i < width; ++i)
      {
        table.columns.push_back("Value " + std::to_string(i + 1));
      }
      for (const auto& values : property.values)
      {
        std::vector<QJsonValue> cells;
        for (double value : values)
        {
          cells.push_back(value);
        }
        cells.resize(width, QJsonValue(QString()));
        table.rows.push_back(std::move(cells));
      }
      groups["ccx.model.materials"].tables.push_back(std::move(table));
    }
  }

  for (const auto& row : sections.get_sections_tree_data())
  {
    SectionsSettings settings;
    if (!sections.get_section_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Sections / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Block", settings.block);
    field(table, "Material", settings.material);
    field(table, "Orientation", settings.orientation);
    field(table, "Thickness", settings.thickness);
    field(table, "Offset", settings.offset);
    field(table, "Beam type", settings.beamType);
    field(table, "Parameter 1", settings.parameter1);
    field(table, "Parameter 2", settings.parameter2);
    field(table, "Parameter 3", settings.parameter3);
    field(table, "Parameter 4", settings.parameter4);
    field(table, "Parameter 5", settings.parameter5);
    field(table, "Parameter 6", settings.parameter6);
    field(table, "X", settings.x);
    field(table, "Y", settings.y);
    field(table, "Z", settings.z);
    field(table, "Offset 1", settings.offset1);
    field(table, "Offset 2", settings.offset2);
    groups["ccx.model.sections"].tables.push_back(std::move(table));
  }

  for (const auto& row : sections.get_orientations_tree_data())
  {
    OrientationsSettings settings;
    if (!sections.get_orientation_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Orientations / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "System", settings.system);
    field(table, "A", settings.a);
    field(table, "B", settings.b);
    field(table, "Axis", settings.axis);
    field(table, "Angle", settings.angle);
    groups["ccx.model.sections"].tables.push_back(std::move(table));
  }

  for (const auto& row : interactions.get_constraints_tree_data())
  {
    ConstraintsSettings settings;
    if (!interactions.get_constraint_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Constraints / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Terms", settings.terms);
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
    field(table, "Tolerance", settings.tolerance);
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }

  for (const auto& row : interactions.get_surfaceinteractions_tree_data())
  {
    SurfaceInteractionsSettings settings;
    if (!interactions.get_surfaceinteraction_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Surface Interactions / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Pressure overclosure", settings.pressureOverclosure);
    field(table, "Conductance", settings.conductance);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Behavior", settings.behavior);
    field(table, "C 0", settings.c0);
    field(table, "P 0", settings.p0);
    field(table, "Slope", settings.slope);
    field(table, "Sigma", settings.sigma);
    field(table, "Linear c 0", settings.linearC0);
    field(table, "Tied slope", settings.tiedSlope);
    field(table, "Conversion", settings.conversion);
    field(table, "Weight", settings.weight);
    field(table, "Velocity", settings.velocity);
    field(table, "Mu", settings.mu);
    field(table, "Lambda value", settings.lambdaValue);
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }

  for (const auto& row : interactions.get_contactpairs_tree_data())
  {
    ContactPairsSettings settings;
    if (!interactions.get_contactpair_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Contact Pairs / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Surface interaction", settings.surfaceInteraction);
    field(table, "Contact type", settings.contactType);
    field(table, "Master", settings.master);
    field(table, "Slave", settings.slave);
    field(table, "Adjust", settings.adjust);
    field(table, "Adjust nodeset", settings.adjustNodeset);
    groups["ccx.model.interactions"].tables.push_back(std::move(table));
  }

  for (const auto& row : amplitudes.get_amplitudes_tree_data())
  {
    AmplitudesSettings settings;
    if (!amplitudes.get_amplitude_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Amplitudes / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Points", settings.points);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Shift x", settings.shiftX);
    field(table, "Shift y", settings.shiftY);
    field(table, "Total time", settings.totalTime);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : bcs.get_bcsdisplacements_tree_data())
  {
    BCsDisplacementsSettings settings;
    if (!bcs.get_displacement_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "B Cs Displacements / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Nodeset", settings.nodeset);
    field(table, "Component 1", settings.component1);
    field(table, "Component 2", settings.component2);
    field(table, "Component 3", settings.component3);
    field(table, "Component 4", settings.component4);
    field(table, "Component 5", settings.component5);
    field(table, "Component 6", settings.component6);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : bcs.get_bcstemperatures_tree_data())
  {
    BCsTemperaturesSettings settings;
    if (!bcs.get_temperature_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "B Cs Temperatures / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Nodeset", settings.nodeset);
    field(table, "Magnitude", settings.magnitude);
    field(table, "Keyword", settings.keyword);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_initialconditions_tree_data())
  {
    InitialConditionsSettings settings;
    if (!steps.get_initialcondition_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Initial Conditions / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Block stress", settings.blockStress);
    field(table, "Element stress", settings.elementStress);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Bc id", settings.bcId);
    field(table, "Condition type", settings.conditionType);
    groups["ccx.model.initial_conditions"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_fieldoutputs_tree_data())
  {
    FieldOutputsSettings settings;
    if (!steps.get_fieldoutput_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Field Outputs / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Parameters", settings.parameters);
    field(table, "Enabled keys", settings.enabledKeys);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    groups["ccx.steps"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_historyoutputs_tree_data())
  {
    HistoryOutputsSettings settings;
    if (!steps.get_historyoutput_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "History Outputs / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Parameters", settings.parameters);
    field(table, "Enabled keys", settings.enabledKeys);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    groups["ccx.steps"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsforces_tree_data())
  {
    LoadsForcesSettings settings;
    if (!loads.get_force_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Forces / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Nodeset", settings.nodeset);
    field(table, "Component 1", settings.component1);
    field(table, "Component 2", settings.component2);
    field(table, "Component 3", settings.component3);
    field(table, "Component 4", settings.component4);
    field(table, "Component 5", settings.component5);
    field(table, "Component 6", settings.component6);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadspressures_tree_data())
  {
    LoadsPressuresSettings settings;
    if (!loads.get_pressure_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Pressures / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    field(table, "Magnitude", settings.magnitude);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsheatfluxes_tree_data())
  {
    LoadsHeatfluxesSettings settings;
    if (!loads.get_heatflux_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Heatfluxes / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    field(table, "Magnitude", settings.magnitude);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsgravity_tree_data())
  {
    LoadsGravitySettings settings;
    if (!loads.get_gravity_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Gravity / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Block", settings.block);
    field(table, "Magnitude", settings.magnitude);
    field(table, "Direction", settings.direction);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadscentrifugal_tree_data())
  {
    LoadsCentrifugalSettings settings;
    if (!loads.get_centrifugal_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Centrifugal / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Block", settings.block);
    field(table, "Magnitude", settings.magnitude);
    field(table, "Coordinate", settings.coordinate);
    field(table, "Direction", settings.direction);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadstrajectory_tree_data())
  {
    LoadsTrajectorySettings settings;
    if (!loads.get_trajectory_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Trajectory / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Samples", settings.samples);
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Load type", settings.loadType);
    field(table, "Curve", settings.curve);
    field(table, "Vertex", settings.vertex);
    field(table, "Surfaces", settings.surfaces);
    field(table, "Direction", settings.direction);
    field(table, "Time begin", settings.timeBegin);
    field(table, "Time end", settings.timeEnd);
    field(table, "Operation", settings.operation);
    field(table, "Model change", settings.modelChange);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsfilm_tree_data())
  {
    LoadsFilmSettings settings;
    if (!loads.get_film_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Film / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    field(table, "Temperature", settings.temperature);
    field(table, "Coefficient", settings.coefficient);
    field(table, "Secondary amplitude", settings.secondaryAmplitude);
    field(table, "Secondary time delay", settings.secondaryTimeDelay);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsradiation_tree_data())
  {
    LoadsRadiationSettings settings;
    if (!loads.get_radiation_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Radiation / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    field(table, "Temperature", settings.temperature);
    field(table, "Coefficient", settings.coefficient);
    field(table, "Secondary amplitude", settings.secondaryAmplitude);
    field(table, "Secondary time delay", settings.secondaryTimeDelay);
    field(table, "Cavity", settings.cavity);
    field(table, "Cavity radiation", settings.cavityRadiation);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadssurfacetraction_tree_data())
  {
    LoadsSurfaceTractionSettings settings;
    if (!loads.get_surfacetraction_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Surface Traction / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Sideset", settings.sideset);
    field(table, "Component 1", settings.component1);
    field(table, "Component 2", settings.component2);
    field(table, "Component 3", settings.component3);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : loads.get_loadsbodyheatflux_tree_data())
  {
    LoadsBodyHeatfluxSettings settings;
    if (!loads.get_bodyheatflux_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Loads Body Heatflux / " + std::to_string(rowId(row));
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Amplitude", settings.amplitude);
    field(table, "Time delay", settings.timeDelay);
    field(table, "Operation", settings.operation);
    field(table, "Magnitude", settings.magnitude);
    field(table, "Entity type", settings.entityType);
    field(table, "Ids", settings.ids);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_physicalconstants_tree_data())
  {
    PhysicalConstantsSettings settings;
    if (!steps.get_physicalconstants_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Physical constants";
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Absolute zero", settings.absoluteZero);
    field(table, "Stefan boltzmann", settings.stefanBoltzmann);
    field(table, "Newton gravity", settings.newtonGravity);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : steps.get_damping_tree_data())
  {
    DampingSettings settings;
    if (!steps.get_damping_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Damping";
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Alpha", settings.alpha);
    field(table, "Beta", settings.beta);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  for (const auto& row : custom.get_customlines_tree_data())
  {
    CustomLinesSettings settings;
    if (!custom.get_customline_settings(rowId(row), settings, error))
    {
      return false;
    }
    ReportTable table;
    table.title = "Custom input lines";
    table.columns = {"Property", "Value"};
    field(table, "Id", settings.id);
    field(table, "Type", settings.type);
    field(table, "Name", settings.name);
    field(table, "Position", settings.position);
    field(table, "Keyword", settings.keyword);
    field(table, "Keyword id", settings.keywordId);
    field(table, "Text", settings.text);
    groups["ccx.other"].tables.push_back(std::move(table));
  }

  groups["ccx.model.sets"].tables.push_back(listing("Node sets", mesh.get_nodeset_tree_data()));
  groups["ccx.model.sets"].tables.push_back(listing("Side sets", mesh.get_sideset_tree_data()));
  int stepOrdinal = 0;
  for (int id : steps.get_steps_ids())
  {
    ++stepOrdinal;
    StepsSettings settings;
    if (!steps.get_step_settings(id, settings, error))
    {
      return false;
    }
    ReportSection step;
    step.id = "ccx.steps." + std::to_string(id);
    step.title = "Analysis step " + std::to_string(stepOrdinal) + " (ID " + std::to_string(id) + "): " + settings.name;
    ReportTable parameters;
    parameters.title = "Procedure and parameters";
    parameters.columns = {"Property", "Value"};
    field(parameters, "type", settings.type);
    field(parameters, "procedure", settings.procedure);
    field(parameters, "parameters", settings.parameters);
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
  groups["ccx.model.overview"].paragraphs.push_back("Current model captured from CalculiX Core. This is not the input of a historical job. No physical units or result values are inferred.");
  ReportDocument candidate = output;
  candidate.source = source;
  // Hash the entire captured report content before display selection is applied.
  // This identifies this reporting snapshot, not an unrecorded solver run.
  QJsonArray captured;
  for (const auto& item : groups)
  {
    captured.append(item.second.to_json());
  }
  candidate.source.snapshot_fingerprint = QCryptographicHash::hash(QJsonDocument(captured).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex().toStdString();
  const auto enabled = [&](const std::string& key)
  {
    const auto found = request.overrides.values.find(key);
    return found != request.overrides.values.end() && found->second == "true";
  };
  std::set<int> selected;
  const auto selection = request.overrides.values.find("steps");
  std::vector<int> selectedIds;
  if (!CalculiXReportImageSettings::select_steps(steps.get_steps_ids(),
        selection == request.overrides.values.end() ? "all" : selection->second, selectedIds, error))
  {
    return false;
  }
  selected.insert(selectedIds.begin(), selectedIds.end());
  auto& stepSections = groups["ccx.steps"].children;
  stepSections.erase(std::remove_if(stepSections.begin(), stepSections.end(), [&](ReportSection& step)
  {
    const int id = QString::fromStdString(step.id).section('.', -1).toInt();
    step.children.erase(std::remove_if(step.children.begin(), step.children.end(), [&](const ReportSection& child)
    {
      const auto key = "ccx.steps." + QString::fromStdString(child.id).section('.', -1).toStdString();
      return !enabled(key);
    }), step.children.end());
    if (!enabled("ccx.steps.parameters"))
    {
      step.tables.clear();
    }
    return !selected.empty() && selected.count(id) == 0;
  }), stepSections.end());
  candidate.sections.clear();
  for (const auto& item : definition.sections)
  {
    if (item.id.rfind("post.", 0) != 0 && enabled(item.id))
    {
      candidate.sections.push_back(std::move(groups[item.id]));
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
