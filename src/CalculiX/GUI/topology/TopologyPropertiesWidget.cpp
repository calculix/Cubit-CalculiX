#include <type_traits>
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
  const QString method = [&]() -> QString {
    if (options.method == TopologyMethod::Density) {
      return "Density";
    } else {
      if (options.method == TopologyMethod::LevelSet) {
        return "Level set";
      } else {
        return "Not set";
      }
    }
  }();
  addValue(settingsGroup, "Method", method);
  const char* objective = topology_objective_name(options.objective);
  addValue(settingsGroup, "Objective", [&]() -> std::common_type_t<decltype(QString()), decltype(QString::fromLatin1(objective))> {
    if (objective == nullptr) {
      return QString();
    } else {
      return QString::fromLatin1(objective);
    }
  }());
  addValue(settingsGroup, "Volume fraction", [&]() -> std::common_type_t<decltype(QString::number(options.volume_fraction, 'g', 17)), decltype(QString())> {
    if (options.has_volume_fraction) {
      return QString::number(options.volume_fraction, 'g', 17);
    } else {
      return QString();
    }
  }());
  addValue(settingsGroup, "Void stiffness ratio", [&]() -> std::common_type_t<decltype(QString::number(options.void_ratio, 'g', 17)), decltype(QString())> {
    if (options.has_void_ratio) {
      return QString::number(options.void_ratio, 'g', 17);
    } else {
      return QString();
    }
  }());
  addValue(settingsGroup, "Material cap", [&]() -> std::common_type_t<decltype(QString::number(options.material_cap)), decltype(QString())> {
    if (options.has_material_cap) {
      return QString::number(options.material_cap);
    } else {
      return QString();
    }
  }());
  addValue(settingsGroup, "Maximum iterations", [&]() -> std::common_type_t<decltype(QString::number(options.maximum_iterations)), decltype(QString())> {
    if (options.has_maximum_iterations) {
      return QString::number(options.maximum_iterations);
    } else {
      return QString();
    }
  }());
  if (options.method == TopologyMethod::Density)
  {
    addValue(settingsGroup, "Filter radius", [&]() -> std::common_type_t<decltype(QString::number(options.filter_radius, 'g', 17)), decltype(QString())> {
      if (options.has_filter_radius) {
        return QString::number(options.filter_radius, 'g', 17);
      } else {
        return QString();
      }
    }());
    addValue(settingsGroup, "Density move limit", [&]() -> std::common_type_t<decltype(QString::number(options.move_limit, 'g', 17)), decltype(QString())> {
      if (options.has_move_limit) {
        return QString::number(options.move_limit, 'g', 17);
      } else {
        return QString();
      }
    }());
    addValue(settingsGroup, "Adaptive density moves", [&]() -> std::common_type_t<std::common_type_t<decltype(QStringLiteral("Yes")), decltype(QStringLiteral("No"))>, decltype(QString())> {
      if (options.has_adaptive_move_limits) {
        return ([&]() -> std::common_type_t<decltype(QStringLiteral("Yes")), decltype(QStringLiteral("No"))> {
          if (options.adaptive_move_limits) {
            return QStringLiteral("Yes");
          } else {
            return QStringLiteral("No");
          }
        }());
      } else {
        return QString();
      }
    }());
    addValue(settingsGroup, "Minimum move limit", [&]() -> std::common_type_t<decltype(QString::number(options.minimum_move_limit, 'g', 17)), decltype(QString())> {
      if (options.has_minimum_move_limit) {
        return QString::number(options.minimum_move_limit, 'g', 17);
      } else {
        return QString();
      }
    }());
    addValue(settingsGroup, "Density tolerance", [&]() -> std::common_type_t<decltype(QString::number(options.density_tolerance, 'g', 17)), decltype(QString())> {
      if (options.has_density_tolerance) {
        return QString::number(options.density_tolerance, 'g', 17);
      } else {
        return QString();
      }
    }());
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
  addValue(settingsGroup, "Relative objective tolerance", [&]() -> std::common_type_t<decltype(QString::number(options.compliance_tolerance, 'g', 17)), decltype(QString())> {
    if (options.has_compliance_tolerance) {
      return QString::number(options.compliance_tolerance, 'g', 17);
    } else {
      return QString();
    }
  }());
  addReference(settingsGroup, PropertyReferenceKind::Nodeset, "Displacement nodeset", [&]() -> std::common_type_t<decltype(QString::number(options.displacement_nodeset)), decltype(QString())> {
    if (options.has_displacement_nodeset) {
      return QString::number(options.displacement_nodeset);
    } else {
      return QString();
    }
  }());
  addValue(settingsGroup, "Displacement component", [&]() -> std::common_type_t<decltype(QString::number(options.displacement_component)), decltype(QString())> {
    if (options.has_displacement_component) {
      return QString::number(options.displacement_component);
    } else {
      return QString();
    }
  }());
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
