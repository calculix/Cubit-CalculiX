#include "TopologyPropertiesWidget.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"

TopologyPropertiesWidget::TopologyPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool TopologyPropertiesWidget::setTopology(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXTopologyInterface core;
  TopologyOptions options;
  if (!core.get_topology_settings(id, options))
  {
    PRINT_ERROR("Cannot load topology properties %d.\n", id);
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Topology", id);
  addEditableName(general, id, QString::fromStdString(options.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  const QString method = options.method == TopologyMethod::Density ? "Density" : (options.method == TopologyMethod::LevelSet ? "Level set" : "Not set");
  addValue(settingsGroup, "Method", method);
  const char* objective = topology_objective_name(options.objective);
  addValue(settingsGroup, "Objective", objective == nullptr ? QString() : QString::fromLatin1(objective));
  addValue(settingsGroup, "Volume fraction", options.has_volume_fraction ? QString::number(options.volume_fraction, 'g', 17) : QString());
  addValue(settingsGroup, "Void stiffness ratio", options.has_void_ratio ? QString::number(options.void_ratio, 'g', 17) : QString());
  addValue(settingsGroup, "Material cap", options.has_material_cap ? QString::number(options.material_cap) : QString());
  addValue(settingsGroup, "Maximum iterations", options.has_maximum_iterations ? QString::number(options.maximum_iterations) : QString());
  if (options.method == TopologyMethod::Density)
  {
    addValue(settingsGroup, "Filter radius", options.has_filter_radius ? QString::number(options.filter_radius, 'g', 17) : QString());
    addValue(settingsGroup, "Density move limit", options.has_move_limit ? QString::number(options.move_limit, 'g', 17) : QString());
    addValue(settingsGroup, "Adaptive density moves", options.has_adaptive_move_limits ? (options.adaptive_move_limits ? QStringLiteral("Yes") : QStringLiteral("No")) : QString());
    addValue(settingsGroup, "Minimum move limit", options.has_minimum_move_limit ? QString::number(options.minimum_move_limit, 'g', 17) : QString());
    addValue(settingsGroup, "Density tolerance", options.has_density_tolerance ? QString::number(options.density_tolerance, 'g', 17) : QString());
  }
  else if (options.method == TopologyMethod::LevelSet)
  {
    auto* levelSet = addGroup("level_set", "Level-set settings", PropertySection::Settings);
    addValue(levelSet, "Interface width", QString::number(options.level_set_settings.interface_width, 'g', 17));
    addValue(levelSet, "Regularization length", QString::number(options.level_set_settings.regularization_length, 'g', 17));
    addValue(levelSet, "Field move limit", QString::number(options.level_set_settings.move_limit, 'g', 17));
    addValue(levelSet, "Field tolerance", QString::number(options.level_set_settings.field_tolerance, 'g', 17));
    addValue(levelSet, "Seed spacing", QString::number(options.level_set_settings.seed_spacing, 'g', 17));
    addValue(levelSet, "Seed radius", QString::number(options.level_set_settings.seed_radius, 'g', 17));
    addValue(levelSet, "Integration tolerance", QString::number(options.level_set_settings.integration_tolerance, 'g', 17));
    addValue(levelSet, "Maximum integration refinements", QString::number(options.level_set_settings.integration_max_refinements));
  }
  addValue(settingsGroup, "Relative objective tolerance", options.has_compliance_tolerance ? QString::number(options.compliance_tolerance, 'g', 17) : QString());
  addReference(settingsGroup, PropertyReferenceKind::Nodeset, "Displacement nodeset", options.has_displacement_nodeset ? QString::number(options.displacement_nodeset) : QString());
  addValue(settingsGroup, "Displacement component", options.has_displacement_component ? QString::number(options.displacement_component) : QString());
  auto* run = addGroup("execution", "Execution", PropertySection::Settings);
  addValue(run, "State", QString::fromStdString(core.get_topology_run_state(id)));
  addValue(run, "Iterations", QString::number(static_cast<qulonglong>(core.get_topology_iteration_data(id).size())));
  return true;
}

void TopologyPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXTopologyInterface core;
  TopologyOptions options;
  if (!core.get_topology_settings(request.entityId, options))
  {
    PRINT_ERROR("Cannot reload the selected topology for renaming.\n");
    return;
  }
  const QString command = QString("ccx modify topology %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(options.name), command, [this, id = request.entityId]() { return setTopology(id); });
}
