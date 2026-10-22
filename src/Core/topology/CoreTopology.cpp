#include "Core/topology/CoreTopology.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include <string>
#include <vector>
#include <map>
#include <array>
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitMessage.hpp"
#include "HDF5Tool.hpp"
#include "TopologyHdf5.hpp"
#include "TopologyIterationIds.hpp"

#include <cmath>
#include <set>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <unordered_map>

// HELPERS
namespace
{
void ApplyTopologyOptions(
    TopologyCase& configuration,
    const TopologyOptions& options)
{
  if (options.has_name)
  {
    configuration.name = options.name;
  }

  if (options.has_method)
  {
    configuration.method = options.method;
  }

  if (options.has_objective)
  {
    configuration.objective = options.objective;
  }

  if (options.has_volume_fraction)
  {
    configuration.target_volume_fraction = options.volume_fraction;
  }

  if (options.has_void_ratio)
  {
    configuration.void_stiffness_ratio = options.void_ratio;
  }

  if (options.has_material_cap)
  {
    configuration.material_cap = options.material_cap;
  }

  if (options.has_maximum_iterations)
  {
    configuration.maximum_iterations = options.maximum_iterations;
  }
  
  if (options.has_filter_radius)
  {
    configuration.filter_radius = options.filter_radius;
  }

  if (options.has_move_limit)
  {
    configuration.move_limit = options.move_limit;
  }

  if (options.has_adaptive_move_limits)
  {
    configuration.adaptive_move_limits = options.adaptive_move_limits;
  }

  if (options.has_minimum_move_limit)
  {
    configuration.minimum_move_limit = options.minimum_move_limit;
  }

  if (options.has_density_tolerance)
  {
    configuration.density_tolerance = options.density_tolerance;
  }

  if (options.has_compliance_tolerance)
  {
    configuration.compliance_tolerance = options.compliance_tolerance;
  }

  if (options.has_displacement_nodeset)
  {
    configuration.displacement_nodeset = options.displacement_nodeset;
  }

  if (options.has_displacement_component)
  {
    configuration.displacement_component = options.displacement_component;
  }
}

void SetTopologyOptimizerDefaults(TopologyCase& configuration)
{
  configuration.move_limit = 0.2;
  configuration.adaptive_move_limits = true;
  configuration.minimum_move_limit = 0.0;
  configuration.density_tolerance = 0.01;
  configuration.compliance_tolerance = 0.001;
  configuration.displacement_nodeset = 0;
  configuration.displacement_component = 0;
}

bool ValidateTopologyConfiguration(const TopologyCase& configuration)
{
  const bool valid_method =
    configuration.method == TopologyMethod::Density ||
    configuration.method == TopologyMethod::LevelSet;

  const bool valid_objective =
      configuration.objective == TopologyObjective::Compliance ||
      configuration.objective == TopologyObjective::Mass ||
      configuration.objective == TopologyObjective::Displacement;

  if (!valid_method || !valid_objective)
  {
    return false;
  }

  return
      configuration.name.find_first_not_of(" \t\r\n") != std::string::npos &&
      std::isfinite(configuration.target_volume_fraction) &&
      configuration.target_volume_fraction > 0.0 &&
      configuration.target_volume_fraction <= 1.0 &&
      std::isfinite(configuration.void_stiffness_ratio) &&
      configuration.void_stiffness_ratio > 0.0 &&
      configuration.void_stiffness_ratio < 1.0 &&
      configuration.maximum_iterations > 0 &&
      configuration.material_cap >= 2 &&
      std::isfinite(configuration.minimum_density) &&
      configuration.minimum_density > 0.0 &&
      configuration.minimum_density < 1.0 &&
      std::isfinite(configuration.filter_radius) &&
      configuration.filter_radius >= 0.0 &&
      std::isfinite(configuration.move_limit) &&
      configuration.move_limit > 0.0 &&
      configuration.move_limit <= 1.0 &&
      std::isfinite(configuration.minimum_move_limit) &&
      configuration.minimum_move_limit >= 0.0 &&
      configuration.minimum_move_limit <= configuration.move_limit &&
      std::isfinite(configuration.density_tolerance) &&
      configuration.density_tolerance > 0.0 &&
      configuration.density_tolerance <= 1.0 &&
      std::isfinite(configuration.compliance_tolerance) &&
      configuration.compliance_tolerance > 0.0 &&
      configuration.compliance_tolerance <= 1.0 &&
      configuration.displacement_nodeset >= 0 &&
      configuration.displacement_component >= 0 &&
      configuration.displacement_component <= 3;
}
}
// HELPERS END

CoreTopology::CoreTopology()
{
}

CoreTopology::~CoreTopology()
{
}

bool CoreTopology::init()
{
  if (is_initialized)
  {
    return false;
  }

  is_initialized = true;
  return true;
}

bool CoreTopology::update()
{
  // Case configuration currently changes only through explicit methods.
  return is_initialized;
}

bool CoreTopology::reset()
{
  topology_cases.clear();
  return true;
}

bool CoreTopology::check_initialized()
{
  return is_initialized;
}

bool CoreTopology::save_configuration(HDF5Tool& file) const
{
  try
  {
    const std::string root = "Cubit-CalculiX/Topology";

    if (topology_cases.size() > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
      PRINT_ERROR("Too many topology cases to save.\n");
      return false;
    }

    if (file.nameExists(root))
    {
      if (!file.deleteGroup(root))
      {
        PRINT_ERROR("Unable to replace saved topology data.\n");
        return false;
      }
    }

    if (!file.createGroup(root) ||
        !file.write_dataset_int_rank_1("version", root, {8}) ||
        !file.write_dataset_int_rank_1("case_count", root,
            {static_cast<int>(topology_cases.size())}))
    {
      PRINT_ERROR("Unable to create topology save data.\n");
      return false;
    }

    std::vector<int> case_ids;

    for (const auto& entry : topology_cases)
    {
      const int id = entry.first;
      const auto& configuration = entry.second;

      if (id <= 0 ||
          !ValidateTopologyConfiguration(configuration) ||
          !std::isfinite(configuration.density_penalization) ||
          configuration.density_penalization < 1.0)
      {
        PRINT_ERROR("Invalid topology configuration %d.\n", id);
        return false;
      }

      case_ids.push_back(id);

      const std::string group = root + "/Case_" + std::to_string(id);

      const std::vector<int> integers = {
          static_cast<int>(configuration.method),
          static_cast<int>(configuration.objective),
          configuration.maximum_iterations,
          configuration.displacement_nodeset,
          configuration.displacement_component,
          configuration.adaptive_move_limits ? 1 : 0,
          configuration.material_cap
      };

      const std::vector<double> parameters = {
          configuration.target_volume_fraction,
          configuration.void_stiffness_ratio,
          configuration.density_penalization,
          configuration.minimum_density,
          configuration.filter_radius,
          configuration.move_limit,
          configuration.density_tolerance,
          configuration.compliance_tolerance,
          configuration.minimum_move_limit
      };

      // Row: operation (1=add, 0=remove), set type, set ID.
      std::vector<std::vector<int>> preserved_sets;

      for (const auto& selection : configuration.added_preserved_sets)
      {
        for (const int set_id : selection.second)
        {
          preserved_sets.push_back({1, selection.first, set_id});
        }
      }

      for (const auto& selection : configuration.removed_preserved_sets)
      {
        for (const int set_id : selection.second)
        {
          preserved_sets.push_back({0, selection.first, set_id});
        }
      }

      if (!file.createGroup(group) ||
          !file.write_dataset_string_rank_1("name", group, {configuration.name}) ||
          !file.write_dataset_int_rank_1("integers", group, integers) ||
          !file.write_dataset_double_rank_1("parameters", group, parameters))
      {
        PRINT_ERROR("Unable to save topology case %d.\n", id);
        return false;
      }

      // Existing HDF5 helpers need no dataset for empty selections.
      if (!preserved_sets.empty())
      {
        if (!file.write_dataset_int_rank_2("preserved_sets", group, preserved_sets))
        {
          PRINT_ERROR("Unable to save preserved sets for topology %d.\n", id);
          return false;
        }
      }

      TopologyHdf5<false> archive(file, group);
      if (!archive.value("runtime", configuration))
      {
        PRINT_ERROR("Unable to save topology %d iteration data.\n", id);
        return false;
      }
    }

    if (!case_ids.empty())
    {
      if (!file.write_dataset_int_rank_1("case_ids", root, case_ids))
      {
        PRINT_ERROR("Unable to save topology case IDs.\n");
        return false;
      }
    }

    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR(
        "Unable to save topology configurations: %s\n",
        error.getDetailMsg().c_str());
    return false;
  }
}

bool CoreTopology::read_configuration(HDF5Tool& file)
{
  try
  {
    const std::string root = "Cubit-CalculiX/Topology";

    // Older files contain no topology configuration.
    if (!file.nameExists(root))
    {
      topology_cases.clear();
      return true;
    }

    std::vector<int> version;
    std::vector<int> count;
    std::vector<int> case_ids;

    if (!file.read_dataset_int_rank_1("version", root, version) ||
        version.size() != 1 ||
        (version[0] < 1 || version[0] > 8) ||
        !file.read_dataset_int_rank_1("case_count", root, count) ||
        count.size() != 1 || count[0] < 0)
    {
      PRINT_ERROR("Invalid topology save format.\n");
      return false;
    }

    if (count[0] > 0)
    {
      if (!file.read_dataset_int_rank_1("case_ids", root, case_ids) ||
          case_ids.size() != static_cast<size_t>(count[0]))
      {
        PRINT_ERROR("Invalid saved topology case IDs.\n");
        return false;
      }
    }

    std::map<int, TopologyCase> loaded;

    for (const int id : case_ids)
    {
      if (id <= 0 || loaded.count(id) != 0)
      {
        PRINT_ERROR("Invalid or duplicate topology ID %d.\n", id);
        return false;
      }

      const std::string group =
          root + "/Case_" + std::to_string(id);

      std::vector<std::string> names;
      std::vector<int> integers;
      std::vector<double> parameters;

      if (!file.nameExists(group) ||
          !file.read_dataset_string_rank_1("name", group, names) ||
          !file.read_dataset_int_rank_1("integers", group, integers) ||
          !file.read_dataset_double_rank_1("parameters", group, parameters) ||
          names.size() != 1 ||
          integers.size() != (version[0] >= 8 ? 7u : (version[0] >= 7 ? 6u : (version[0] >= 5 ? 5u : 3u))) ||
          parameters.size() != (version[0] >= 7 ? 9u : (version[0] >= 4 ? 8u : 5u)))
      {
        PRINT_ERROR("Incomplete saved topology case %d.\n", id);
        return false;
      }

      TopologyCase configuration{};

      configuration.name = names[0];
      configuration.method = static_cast<TopologyMethod>(integers[0]);
      configuration.objective = static_cast<TopologyObjective>(integers[1]);
      configuration.maximum_iterations = integers[2];

      if (version[0] >= 8)
      {
        configuration.material_cap = integers[6];
      }

      configuration.target_volume_fraction = parameters[0];
      configuration.void_stiffness_ratio = parameters[1];
      configuration.density_penalization = parameters[2];
      configuration.minimum_density = parameters[3];
      configuration.filter_radius = parameters[4];

      SetTopologyOptimizerDefaults(configuration);

      if (version[0] >= 5)
      {
        configuration.displacement_nodeset = integers[3];
        configuration.displacement_component = integers[4];
      }

      if (version[0] >= 4)
      {
        configuration.move_limit = parameters[5];
        configuration.density_tolerance = parameters[6];
        configuration.compliance_tolerance = parameters[7];
      }

      if (version[0] >= 7)
      {
        if (integers[5] != 0 && integers[5] != 1)
        {
          PRINT_ERROR("Invalid saved adaptive move setting for topology %d.\n", id);
          return false;
        }

        configuration.adaptive_move_limits = integers[5] == 1;
        configuration.minimum_move_limit = parameters[8];
      }

      if (!ValidateTopologyConfiguration(configuration) ||
          !std::isfinite(configuration.density_penalization) ||
          configuration.density_penalization < 1.0)
      {
        PRINT_ERROR("Invalid saved topology case %d.\n", id);
        return false;
      }

      if (file.nameExists(group + "/preserved_sets"))
      {
        std::vector<std::vector<int>> selections;

        if (!file.read_dataset_int_rank_2("preserved_sets", group, selections))
        {
          PRINT_ERROR("Unable to read preserved sets for topology %d.\n", id);
          return false;
        }

        for (const auto& selection : selections)
        {
          if (selection.size() != 3 ||
              (selection[0] != 0 && selection[0] != 1) ||
              selection[1] < 1 || selection[1] > 3 ||
              selection[2] <= 0)
          {
            PRINT_ERROR("Invalid saved preservation selection for topology %d.\n", id);
            return false;
          }

          auto& target = selection[0] == 1
              ? configuration.added_preserved_sets
              : configuration.removed_preserved_sets;

          if (!target[selection[1]].insert(selection[2]).second)
          {
            PRINT_ERROR("Duplicate saved preservation selection.\n");
            return false;
          }
        }

        for (const auto& selection : configuration.added_preserved_sets)
        {
          const auto removed = configuration.removed_preserved_sets.find(selection.first);

          if (removed == configuration.removed_preserved_sets.end())
          {
            continue;
          }

          for (const int set_id : selection.second)
          {
            if (removed->second.count(set_id) != 0)
            {
              PRINT_ERROR("Conflicting saved preservation selections.\n");
              return false;
            }
          }
        }
      }

      if (version[0] >= 2)
      {
        if (version[0] >= 6 && !file.nameExists(group + "/runtime/adjoint_jobs"))
        {
          PRINT_ERROR("Missing saved adjoint job associations for topology %d.\n", id);
          return false;
        }

        TopologyHdf5<true> archive(file, group);
        if (!archive.value("runtime", configuration))
        {
          PRINT_ERROR("Unable to read topology %d iteration data.\n", id);
          return false;
        }

        if (version[0] == 2)
        {
          if (configuration.selected_iteration == std::numeric_limits<int>::max() ||
              !increment_topology_iteration_keys(configuration.iteration_jobs) ||
              !increment_topology_iteration_keys(configuration.iteration_element_energies) ||
              !increment_topology_iteration_keys(configuration.iteration_total_energies) ||
              !increment_topology_iteration_keys(configuration.iteration_evaluations) ||
              !increment_topology_iteration_keys(configuration.iteration_density_designs))
          {
            PRINT_ERROR("Unable to migrate topology iteration IDs.\n");
            return false;
          }
          if (configuration.selected_iteration >= 0)
          {
            ++configuration.selected_iteration;
          }
        }

        if (!valid_topology_iteration_keys(configuration.iteration_jobs) ||
            !valid_topology_iteration_keys(configuration.iteration_adjoint_jobs) ||
            !valid_topology_iteration_keys(configuration.iteration_element_energies) ||
            !valid_topology_iteration_keys(configuration.iteration_total_energies) ||
            !valid_topology_iteration_keys(configuration.iteration_evaluations) ||
            !valid_topology_iteration_keys(configuration.iteration_density_designs))
        {
          PRINT_ERROR("Invalid saved topology iteration ID.\n");
          return false;
        }

        std::set<int> linked_jobs;

        for (const auto* associations : {&configuration.iteration_jobs, &configuration.iteration_adjoint_jobs})
        {
          for (const auto& iteration : *associations)
          {
            if (iteration.second <= 0 || !linked_jobs.insert(iteration.second).second)
            {
              PRINT_ERROR("Invalid or duplicate saved topology job association.\n");
              return false;
            }
          }
        }

        for (const auto& iteration : configuration.iteration_adjoint_jobs)
        {
          if (configuration.iteration_jobs.count(iteration.first) == 0)
          {
            PRINT_ERROR("Saved adjoint job has no physical iteration job.\n");
            return false;
          }
        }

        if (configuration.selected_iteration < -1 || configuration.selected_iteration == 0)
        {
          PRINT_ERROR("Invalid saved topology iteration selection.\n");
          return false;
        }

        if (configuration.selected_iteration >= 0)
        {
          const int iteration = configuration.selected_iteration;
          if (configuration.iteration_jobs.count(iteration) == 0 ||
              configuration.iteration_evaluations.count(iteration) == 0 ||
              configuration.iteration_density_designs.count(iteration) == 0)
          {
            PRINT_ERROR("Incomplete selected topology iteration.\n");
            return false;
          }
        }

        if (configuration.run_state == TopologyRunState::Running)
        {
          configuration.run_state = TopologyRunState::Failed;
          PRINT_INFO("Topology %d was saved while running; "
                     "restored as an interrupted run.\n", id);
        }
      }
      else
      {
        configuration.selected_iteration = -1;
        configuration.run_state = TopologyRunState::NotRun;
      }

      loaded.emplace(id, std::move(configuration));
    }

    // Commit only after every case has been read successfully.
    topology_cases.swap(loaded);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR(
        "Unable to read topology configurations: %s\n",
        error.getDetailMsg().c_str());
    return false;
  }
}

bool CoreTopology::create_topology(int topology_id,const TopologyOptions& options)
{
  if (!is_initialized || topology_id <= 0)
  {
    PRINT_ERROR("Invalid topology initialization or ID.\n");
    return false;
  }

  if (topology_cases.count(topology_id) != 0)
  {
    PRINT_ERROR("Topology case %d already exists.\n", topology_id);
    return false;
  }

  // Defaults belong to CoreTopology.
  TopologyCase configuration;
  configuration.name = "topology_" + std::to_string(topology_id);
  configuration.method = TopologyMethod::Density;
  configuration.objective = TopologyObjective::Compliance;
  configuration.target_volume_fraction = 0.3;
  configuration.void_stiffness_ratio = 1.0e-3;
  configuration.maximum_iterations = 100;
  configuration.density_penalization = 3.0;
  configuration.minimum_density = 1.0e-3;
  configuration.filter_radius = 0.0;
  SetTopologyOptimizerDefaults(configuration);

  ApplyTopologyOptions(configuration, options);

  if (!ValidateTopologyConfiguration(configuration))
  {
    PRINT_ERROR("Invalid topology case configuration.\n");
    return false;
  }

  topology_cases.emplace(topology_id, configuration);
  return true;
}

bool CoreTopology::modify_topology(
    int topology_id,
    const TopologyOptions& options)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (entry->second.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is running.\n", topology_id);
    return false;
  }

  // Preserve selections, element flags, and iteration records.
  TopologyCase configuration = entry->second;
  ApplyTopologyOptions(configuration, options);

  if (!ValidateTopologyConfiguration(configuration))
  {
    PRINT_ERROR("Invalid topology case configuration.\n");
    return false;
  }

  entry->second = configuration;
  return true;
}

bool CoreTopology::delete_topology(int topology_id)
{
  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  // The coordinating core must remove linked jobs first.
  if (!entry->second.iteration_jobs.empty() || !entry->second.iteration_adjoint_jobs.empty())
  {
    PRINT_ERROR("Topology case %d still has linked jobs.\n",
                topology_id);
    return false;
  }

  topology_cases.erase(entry);
  return true;
}

bool CoreTopology::resolve_preserved_elements(int topology_id, const std::vector<int>& analysis_element_ids, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements)
{
  CalculiXTopologyInterface ccx_topology_iface;

  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (analysis_element_ids.empty())
  {
    PRINT_ERROR("Topology %d has no analysis elements.\n", topology_id);
    return false;
  }

  std::map<int, bool> flags;

  for (int element_id : analysis_element_ids)
  {
    if (element_id <= 0 ||
        !flags.emplace(element_id, false).second)
    {
      PRINT_ERROR("Invalid or duplicate topology element %d.\n",
                  element_id);
      return false;
    }
  }

  const auto& configuration = entry->second;
  auto effective_sets = default_sets;

  for (const auto& group : configuration.added_preserved_sets)
  {
    effective_sets[group.first].insert(
        group.second.begin(), group.second.end());
  }

  for (const auto& group : configuration.removed_preserved_sets)
  {
    for (int set_id : group.second)
    {
      effective_sets[group.first].erase(set_id);
    }
  }

  for (const auto& group : effective_sets)
  {
    const int set_type = group.first;

    if (set_type < 1 || set_type > 3)
    {
      PRINT_ERROR("Invalid default preserved set type %d.\n",
                  set_type);
      return false;
    }

    for (int set_id : group.second)
    {
      std::vector<int> element_ids;

      if (!ccx_topology_iface.get_topology_set_element_ids(
              topology_id, set_type, set_id, element_ids))
      {
        return false;
      }

      for (int element_id : element_ids)
      {
        const auto flag = flags.find(element_id);

        if (flag == flags.end())
        {
          PRINT_ERROR("Unexpected topology element %d.\n", element_id);
          return false;
        }

        flag->second = true;
      }
    }
  }

  for (int element_id : default_elements)
  {
    if (element_id <= 0)
    {
      PRINT_ERROR("Invalid default preserved element %d.\n",
                  element_id);
      return false;
    }

    const auto flag = flags.find(element_id);

    // Only elements participating in this topology analysis are marked.
    if (flag != flags.end())
    {
      flag->second = true;
    }
  }

  // Commit only after all queries succeed.
  entry->second.element_preserved.swap(flags);
  return true;
}

bool CoreTopology::set_iteration_element_energies(int topology_id,int iteration_id,const std::map<int, double>& element_energies)
{
  if (!is_initialized || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology energy storage request.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;

  if (configuration.iteration_jobs.count(iteration_id) == 0 ||
    configuration.iteration_element_energies.count(iteration_id) != 0 ||
    configuration.iteration_total_energies.count(iteration_id) != 0)
  {
    PRINT_ERROR("Missing job or already stored iteration energies.\n");
    return false;
  }

  if (element_energies.empty() || element_energies.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology iteration energies.\n");
    return false;
  }

  double total_energy = 0.0;

  for (const auto& energy : element_energies)
  {
    if (configuration.element_preserved.count(energy.first) == 0 || !std::isfinite(energy.second))
    {
      PRINT_ERROR("Invalid energy for topology element %d.\n", energy.first);
      return false;
    }

    total_energy += energy.second;
    if (!std::isfinite(total_energy))
    {
      PRINT_ERROR("Invalid total energy for topology %d iteration %d.\n", topology_id, iteration_id);
      return false;
    }
  }

  configuration.iteration_element_energies.emplace(iteration_id, element_energies);
  configuration.iteration_total_energies.emplace(iteration_id, total_energy);
  PRINT_INFO("Topology %d iteration %d: total internal energy %.9g.\n", topology_id, iteration_id, total_energy);

  return true;
}

bool CoreTopology::evaluate_compliance(int topology_id,int iteration_id,const std::map<int, double>& stiffness_factors)
{
  if (!is_initialized || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology compliance evaluation request.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;
  const auto energies = configuration.iteration_element_energies.find(iteration_id);
  const auto total = configuration.iteration_total_energies.find(iteration_id);

  if (configuration.objective != TopologyObjective::Compliance ||
      configuration.iteration_jobs.count(iteration_id) == 0 ||
      energies == configuration.iteration_element_energies.end() ||
      total == configuration.iteration_total_energies.end() ||
      configuration.iteration_evaluations.count(iteration_id) != 0)
  {
    PRINT_ERROR("Missing data or already evaluated topology iteration.\n");
    return false;
  }

  if (energies->second.empty() ||
      energies->second.size() != stiffness_factors.size() ||
      energies->second.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology compliance data.\n");
    return false;
  }

  TopologyIterationEvaluation evaluation;
  evaluation.objective = TopologyObjective::Compliance;
  evaluation.value = 2.0 * total->second;

  if (!std::isfinite(evaluation.value) || evaluation.value < 0.0)
  {
    PRINT_ERROR("Invalid topology compliance value.\n");
    return false;
  }

  for (const auto& energy : energies->second)
  {
    const auto factor = stiffness_factors.find(energy.first);
    const auto preserved = configuration.element_preserved.find(energy.first);

    if (factor == stiffness_factors.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(factor->second) ||
        factor->second <= 0.0 ||
        factor->second > 1.0 ||
        !std::isfinite(energy.second) ||
        energy.second < 0.0)
    {
      PRINT_ERROR("Invalid compliance data for element %d.\n", energy.first);
      return false;
    }

    double sensitivity = 0.0;

    if (!preserved->second)
    {
      sensitivity = -2.0 * energy.second / factor->second;
    }

    if (!std::isfinite(sensitivity))
    {
      PRINT_ERROR("Invalid compliance sensitivity for element %d.\n", energy.first);
      return false;
    }

    evaluation.stiffness_sensitivities.emplace(energy.first, sensitivity);
  }

  /*
  s(ρ) = v + (1 − v)ρᵖ
  dC/dρ = (dC/ds) × (1 − v)pρᵖ⁻¹
  */
  const auto density_design = configuration.iteration_density_designs.find(iteration_id);

  if (density_design != configuration.iteration_density_designs.end())
  {
    const auto& design = density_design->second;

    if (design.element_densities.size() !=
            evaluation.stiffness_sensitivities.size() ||
        !std::isfinite(design.penalization) ||
        design.penalization < 1.0 ||
        !std::isfinite(design.void_ratio) ||
        design.void_ratio <= 0.0 ||
        design.void_ratio >= 1.0)
    {
      PRINT_ERROR("Invalid stored topology density design.\n");
      return false;
    }

    for (const auto& density : design.element_densities)
    {
      const auto stiffness = evaluation.stiffness_sensitivities.find(density.first);
      const auto preserved = configuration.element_preserved.find(density.first);

      if (stiffness == evaluation.stiffness_sensitivities.end() ||
          preserved == configuration.element_preserved.end() ||
          !std::isfinite(density.second) ||
          density.second < 0.0 ||
          density.second > 1.0)
      {
        PRINT_ERROR("Invalid density sensitivity data for element %d.\n", density.first);
        return false;
      }

      double sensitivity = 0.0;

      if (!preserved->second)
      {
        // Handle p == 1 explicitly, including rho == 0.
        double power = 1.0;

        if (design.penalization > 1.0)
        {
          power = std::pow(density.second, design.penalization - 1.0);
        }

        const double factor_derivative = (1.0 - design.void_ratio) * design.penalization * power;

        // Applied factors match the solver; the continuous SIMP derivative
        // remains a sensitivity approximation when materials are grouped.
        sensitivity = stiffness->second * factor_derivative;
      }

      if (!std::isfinite(sensitivity))
      {
        PRINT_ERROR("Invalid density sensitivity for element %d.\n", density.first);
        return false;
      }

      evaluation.density_sensitivities.emplace(density.first, sensitivity);
    }
  }

  configuration.iteration_evaluations.emplace(iteration_id, evaluation);
  PRINT_INFO("Topology %d iteration %d: compliance %.9g.\n", topology_id, iteration_id, evaluation.value);

  return true;
}

bool CoreTopology::evaluate_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals, const std::map<int, double>& stiffness_factors, bool grouped_materials)
{
  if (!is_initialized || iteration_id < 1 || !std::isfinite(value) || value < 0.0 || node_id <= 0)
  {
    PRINT_ERROR("Invalid topology displacement evaluation request.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;
  const auto density_design = configuration.iteration_density_designs.find(iteration_id);

  if (configuration.method != TopologyMethod::Density ||
      configuration.objective != TopologyObjective::Displacement ||
      configuration.displacement_nodeset <= 0 ||
      configuration.displacement_component < 0 ||
      configuration.displacement_component > 3 ||
      configuration.iteration_jobs.count(iteration_id) == 0 ||
      configuration.iteration_evaluations.count(iteration_id) != 0 ||
      density_design == configuration.iteration_density_designs.end())
  {
    PRINT_ERROR("Missing data, invalid objective or already evaluated topology iteration.\n");
    return false;
  }

  if (value > 0.0 && configuration.iteration_adjoint_jobs.count(iteration_id) == 0)
  {
    PRINT_ERROR("Missing topology adjoint job association.\n");
    return false;
  }

  const auto& design = density_design->second;

  if (element_integrals.empty() ||
      element_integrals.size() != stiffness_factors.size() ||
      element_integrals.size() != configuration.element_preserved.size() ||
      element_integrals.size() != design.element_densities.size() ||
      !std::isfinite(design.penalization) ||
      design.penalization < 1.0 ||
      !std::isfinite(design.void_ratio) ||
      design.void_ratio <= 0.0 ||
      design.void_ratio >= 1.0 ||
      !std::isfinite(design.minimum_density) ||
      design.minimum_density <= 0.0 ||
      design.minimum_density >= 1.0)
  {
    PRINT_ERROR("Incomplete displacement data or invalid stored density design.\n");
    return false;
  }

  TopologyIterationEvaluation evaluation;
  evaluation.objective = TopologyObjective::Displacement;
  evaluation.value = value;
  evaluation.displacement_available = true;
  evaluation.displacement_value = value;
  evaluation.displacement_node = node_id;
  evaluation.displacement_nodeset = configuration.displacement_nodeset;
  evaluation.displacement_component = configuration.displacement_component;

  for (const auto& integral : element_integrals)
  {
    const int element_id = integral.first;
    const auto factor = stiffness_factors.find(element_id);
    const auto density = design.element_densities.find(element_id);
    const auto preserved = configuration.element_preserved.find(element_id);

    if (factor == stiffness_factors.end() ||
        density == design.element_densities.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(integral.second) ||
        !std::isfinite(factor->second) ||
        factor->second <= 0.0 ||
        factor->second > 1.0 ||
        !std::isfinite(density->second) ||
        density->second < design.minimum_density ||
        density->second > 1.0)
    {
      PRINT_ERROR("Invalid displacement sensitivity data for element %d.\n", element_id);
      return false;
    }

    const double rho = density->second;
    const double expected_factor = design.void_ratio + (1.0 - design.void_ratio) * std::pow(rho, design.penalization);

    if ((!grouped_materials && std::abs(factor->second - expected_factor) > 1.0e-12) ||
        (preserved->second && (rho != 1.0 || factor->second != 1.0)) ||
        (value == 0.0 && integral.second != 0.0))
    {
      PRINT_ERROR("Inconsistent displacement design data for element %d.\n", element_id);
      return false;
    }

    double stiffness_sensitivity = 0.0;
    double density_sensitivity = 0.0;

    if (!preserved->second)
    {
      stiffness_sensitivity = -integral.second / factor->second;

      const double power = design.penalization == 1.0 ? 1.0 : std::pow(rho, design.penalization - 1.0);
      const double factor_derivative = (1.0 - design.void_ratio) * design.penalization * power;

      // Grouped solves use the continuous SIMP derivative as a surrogate.
      density_sensitivity = stiffness_sensitivity * factor_derivative;
    }

    if (!std::isfinite(stiffness_sensitivity) || !std::isfinite(density_sensitivity))
    {
      PRINT_ERROR("Non-finite displacement sensitivity for element %d.\n", element_id);
      return false;
    }

    evaluation.stiffness_sensitivities.emplace(element_id, stiffness_sensitivity);
    evaluation.density_sensitivities.emplace(element_id, density_sensitivity);
  }

  configuration.iteration_evaluations.emplace(iteration_id, std::move(evaluation));
  PRINT_INFO("Topology %d iteration %d: displacement objective %.9g.\n", topology_id, iteration_id, value);
  return true;
}

bool CoreTopology::build_density_design(int topology_id,int iteration_id,const std::map<int, double>& densities,TopologyDensityDesign& design,std::map<int, double>& stiffness_factors)
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || iteration_id < 1 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology density request.\n");
    return false;
  }

  const auto& configuration = entry->second;

  if (configuration.method != TopologyMethod::Density ||
      configuration.iteration_jobs.count(iteration_id) != 0 ||
      configuration.iteration_density_designs.count(iteration_id) != 0)
  {
    PRINT_ERROR("Density method required or iteration already exists.\n");
    return false;
  }

  if (densities.empty() ||
      densities.size() != configuration.element_preserved.size() ||
      !std::isfinite(configuration.density_penalization) ||
      configuration.density_penalization < 1.0 ||
      !std::isfinite(configuration.void_stiffness_ratio) ||
      configuration.void_stiffness_ratio <= 0.0 ||
      configuration.void_stiffness_ratio >= 1.0 ||
      !std::isfinite(configuration.minimum_density) ||
      configuration.minimum_density <= 0.0 ||
      configuration.minimum_density >= 1.0)
  {
    PRINT_ERROR("Invalid density data or interpolation settings.\n");
    return false;
  }

  TopologyDensityDesign candidate;
  candidate.penalization = configuration.density_penalization;
  candidate.void_ratio = configuration.void_stiffness_ratio;
  candidate.minimum_density = configuration.minimum_density;
  std::map<int, double> factors;

  for (const auto& density : densities)
  {
    const auto preserved = configuration.element_preserved.find(density.first);

    if (preserved == configuration.element_preserved.end() ||
        !std::isfinite(density.second) ||
        density.second < 0.0 || density.second > 1.0)
    {
      PRINT_ERROR("Invalid density for element %d.\n", density.first);
      return false;
    }

    if (!preserved->second && density.second < candidate.minimum_density)
    {
      PRINT_ERROR("Density for element %d is below the minimum %.9g.\n", density.first, candidate.minimum_density);
      return false;
    }

    const double rho = preserved->second ? 1.0 : density.second;

    const double factor = candidate.void_ratio + (1.0 - candidate.void_ratio) * std::pow(rho, candidate.penalization);

    if (!std::isfinite(factor) || factor <= 0.0 || factor > 1.0)
    {
      PRINT_ERROR("Invalid stiffness factor for element %d.\n", density.first);
      return false;
    }

    candidate.element_densities.emplace(density.first, rho);
    factors.emplace(density.first, factor);
  }

  design = std::move(candidate);
  stiffness_factors.swap(factors);
  return true;
}

bool CoreTopology::evaluate_density_volume(int topology_id,int iteration_id,const std::map<int, double>& element_volumes)
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || iteration_id < 1 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology volume evaluation request.\n");
    return false;
  }

  auto& configuration = entry->second;
  const auto design = configuration.iteration_density_designs.find(iteration_id);

  if (design == configuration.iteration_density_designs.end())
  {
    PRINT_ERROR("Missing topology density design.\n");
    return false;
  }

  const auto& densities = design->second.element_densities;
  const double minimum_density = design->second.minimum_density;

  if (!std::isfinite(minimum_density) ||
      minimum_density <= 0.0 ||
      minimum_density >= 1.0)
  {
    PRINT_ERROR("Invalid stored minimum density.\n");
    return false;
  }

  if (element_volumes.empty() ||
      element_volumes.size() != densities.size() ||
      element_volumes.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology element volumes.\n");
    return false;
  }

  TopologyVolumeEvaluation evaluation;

  for (const auto& element : element_volumes)
  {
    const auto density = densities.find(element.first);
    const auto preserved = configuration.element_preserved.find(element.first);

    if (density == densities.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(element.second) ||
        element.second <= 0.0 ||
        !std::isfinite(density->second) ||
        density->second < minimum_density ||
        density->second > 1.0)
    {
      PRINT_ERROR("Invalid volume data for element %d.\n", element.first);
      return false;
    }

    if (preserved->second && density->second != 1.0)
    {
      PRINT_ERROR("Preserved element %d must have density 1.\n", element.first);
      return false;
    }

    evaluation.total_volume += element.second;
    evaluation.solid_volume += density->second * element.second;

    if (preserved->second)
    {
      evaluation.preserved_volume += element.second;
    }
  }

  if (!std::isfinite(evaluation.total_volume) ||
      evaluation.total_volume <= 0.0 ||
      !std::isfinite(evaluation.solid_volume) ||
      !std::isfinite(evaluation.preserved_volume))
  {
    PRINT_ERROR("Invalid accumulated topology volumes.\n");
    return false;
  }

  evaluation.volume_fraction = evaluation.solid_volume / evaluation.total_volume;
  /*
    minimum fraction = preserved fraction + minimum density × free fraction
  */
  evaluation.minimum_volume_fraction = evaluation.preserved_volume / evaluation.total_volume;
  evaluation.minimum_design_volume_fraction = evaluation.minimum_volume_fraction + minimum_density * (1.0 - evaluation.minimum_volume_fraction);

  for (const auto& element : element_volumes)
  {
    const bool preserved = configuration.element_preserved.find(element.first)->second;

    const double sensitivity = preserved ? 0.0 : element.second / evaluation.total_volume;

    evaluation.density_sensitivities.emplace(element.first, sensitivity);
  }

  evaluation.available = true;
  design->second.volume = std::move(evaluation);

  PRINT_INFO(
      "Topology %d iteration %d: volume fraction %.9g; "
      "preserved minimum %.9g.\n",
      topology_id, iteration_id,
      design->second.volume.volume_fraction,
      design->second.volume.minimum_volume_fraction);

  return true;
}

bool CoreTopology::select_compliance_move_limit(const TopologyCase& configuration, int source_iteration, double& move_limit, bool& correlation_available, double& correlation) const
{
  move_limit = configuration.move_limit;
  correlation_available = false;
  correlation = 0.0;

  const auto current = configuration.iteration_density_designs.find(source_iteration);

  if (configuration.method != TopologyMethod::Density ||
      configuration.objective != TopologyObjective::Compliance ||
      current == configuration.iteration_density_designs.end() ||
      !std::isfinite(configuration.move_limit) ||
      configuration.move_limit <= 0.0 ||
      configuration.move_limit > 1.0 ||
      !std::isfinite(configuration.minimum_move_limit) ||
      configuration.minimum_move_limit < 0.0 ||
      configuration.minimum_move_limit > configuration.move_limit)
  {
    PRINT_ERROR("Invalid adaptive compliance move request.\n");
    return false;
  }

  if (!configuration.adaptive_move_limits)
  {
    return true;
  }

  // Initial adaptive algorithm settings belong to CoreTopology.
  constexpr double reduction_factor = 0.5;
  constexpr double recovery_factor = 1.1;
  constexpr double reversal_threshold = -0.25;
  constexpr double alignment_threshold = 0.5;
  constexpr double automatic_minimum_ratio = 0.1;
  constexpr double volume_tolerance = 1.0e-6;
  constexpr double minimum_squared_change = 1.0e-24;

  const double minimum_move = configuration.minimum_move_limit > 0.0 ? configuration.minimum_move_limit : automatic_minimum_ratio * configuration.move_limit;

  if (current == configuration.iteration_density_designs.begin())
  {
    return true;
  }

  auto previous = current;
  --previous;

  if (previous == configuration.iteration_density_designs.begin())
  {
    return true;
  }

  auto older = previous;
  --older;

  if (!std::isfinite(configuration.target_volume_fraction) ||
      configuration.target_volume_fraction <= 0.0 ||
      configuration.target_volume_fraction > 1.0)
  {
    PRINT_ERROR("Invalid adaptive compliance volume target.\n");
    return false;
  }

  for (const auto* stored : {&current->second, &previous->second, &older->second})
  {
    if (!stored->volume.available ||
        !std::isfinite(stored->volume.volume_fraction) ||
        stored->volume.volume_fraction < 0.0 ||
        stored->volume.volume_fraction > 1.0)
    {
      PRINT_ERROR("Missing or invalid volume history for adaptive compliance moves.\n");
      return false;
    }

    // Do not interpret the initial volume reduction as oscillation.
    if (stored->volume.volume_fraction > configuration.target_volume_fraction + volume_tolerance)
    {
      return true;
    }
  }

  const auto& current_densities = current->second.element_densities;
  const auto& previous_densities = previous->second.element_densities;
  const auto& older_densities = older->second.element_densities;
  const auto& weights = current->second.volume.density_sensitivities;

  if (current_densities.empty() ||
      current_densities.size() != previous_densities.size() ||
      current_densities.size() != older_densities.size() ||
      current_densities.size() != weights.size() ||
      current_densities.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete density history for adaptive compliance moves.\n");
    return false;
  }

  if (current->second.density_update_available)
  {
    if (!std::isfinite(current->second.effective_move_limit) || current->second.effective_move_limit <= 0.0)
    {
      PRINT_ERROR("Invalid stored effective move limit.\n");
      return false;
    }

    move_limit = std::max(minimum_move, std::min(configuration.move_limit, current->second.effective_move_limit));
  }

  double product = 0.0;
  double current_norm = 0.0;
  double previous_norm = 0.0;

  for (const auto& density : current_densities)
  {
    const int element_id = density.first;
    const auto previous_density = previous_densities.find(element_id);
    const auto older_density = older_densities.find(element_id);
    const auto weight = weights.find(element_id);
    const auto preserved = configuration.element_preserved.find(element_id);

    if (previous_density == previous_densities.end() ||
        older_density == older_densities.end() ||
        weight == weights.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(density.second) ||
        density.second < 0.0 ||
        density.second > 1.0 ||
        !std::isfinite(previous_density->second) ||
        previous_density->second < 0.0 ||
        previous_density->second > 1.0 ||
        !std::isfinite(older_density->second) ||
        older_density->second < 0.0 ||
        older_density->second > 1.0 ||
        !std::isfinite(weight->second))
    {
      PRINT_ERROR("Invalid adaptive move history for element %d.\n", element_id);
      return false;
    }

    if (preserved->second)
    {
      continue;
    }

    if (weight->second <= 0.0 || weight->second > 1.0)
    {
      PRINT_ERROR("Invalid adaptive move volume weight for element %d.\n", element_id);
      return false;
    }

    const double current_change = density.second - previous_density->second;
    const double previous_change = previous_density->second - older_density->second;

    product += weight->second * current_change * previous_change;
    current_norm += weight->second * current_change * current_change;
    previous_norm += weight->second * previous_change * previous_change;
  }

  if (!std::isfinite(product) || !std::isfinite(current_norm) || !std::isfinite(previous_norm))
  {
    PRINT_ERROR("Invalid adaptive density change correlation.\n");
    return false;
  }

  if (current_norm <= minimum_squared_change || previous_norm <= minimum_squared_change)
  {
    return true;
  }

  correlation = product / std::sqrt(current_norm) / std::sqrt(previous_norm);
  correlation = std::max(-1.0, std::min(1.0, correlation));
  correlation_available = true;

  if (correlation < reversal_threshold)
  {
    move_limit = std::max(minimum_move, move_limit * reduction_factor);
  }
  else if (correlation > alignment_threshold)
  {
    move_limit = std::min(configuration.move_limit, move_limit * recovery_factor);
  }

  return true;
}

bool CoreTopology::set_density_update_diagnostics(int topology_id, int source_iteration, TopologyDensityDesign& design) const
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || source_iteration < 1 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology density update diagnostic request.\n");
    return false;
  }

  const auto& configuration = entry->second;

  if (configuration.method != TopologyMethod::Density ||
      configuration.iteration_density_designs.count(source_iteration) == 0)
  {
    PRINT_ERROR("Missing source density design for update diagnostics.\n");
    return false;
  }

  // Adaptive moves currently apply only to compliance.
  if (configuration.objective == TopologyObjective::Displacement)
  {
    return true;
  }

  double move_limit = configuration.move_limit;
  bool correlation_available = false;
  double correlation = 0.0;

  if (!select_compliance_move_limit(configuration, source_iteration, move_limit, correlation_available, correlation))
  {
    return false;
  }

  design.density_update_available = true;
  design.effective_move_limit = move_limit;
  design.update_correlation_available = correlation_available;
  design.update_correlation = correlation;
  return true;
}

bool CoreTopology::build_density_update(int topology_id,int source_iteration,std::map<int, double>& densities)
{
  densities.clear();

  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || source_iteration < 1 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology density update request.\n");
    return false;
  }

  const auto& configuration = entry->second;
  const auto design = configuration.iteration_density_designs.find(source_iteration);
  const auto evaluation = configuration.iteration_evaluations.find(source_iteration);

  if (configuration.method != TopologyMethod::Density ||
      (configuration.objective != TopologyObjective::Compliance &&
       configuration.objective != TopologyObjective::Displacement) ||
      design == configuration.iteration_density_designs.end() ||
      evaluation == configuration.iteration_evaluations.end() ||
      evaluation->second.objective != configuration.objective ||
      !design->second.volume.available)
  {
    PRINT_ERROR("Missing or inconsistent evaluated density iteration.\n");
    return false;
  }

  const auto& previous = design->second.element_densities;
  const auto& gradients = evaluation->second.filtered_density_sensitivities;
  const auto& volume = design->second.volume;
  const double minimum_density = design->second.minimum_density;

  if (previous.empty() ||
    previous.size() != gradients.size() ||
    previous.size() != volume.density_sensitivities.size() ||
    previous.size() != configuration.element_preserved.size() ||
    !std::isfinite(configuration.target_volume_fraction) ||
    configuration.target_volume_fraction <= 0.0 ||
    configuration.target_volume_fraction > 1.0 ||
    !std::isfinite(minimum_density) ||
    minimum_density <= 0.0 ||
    minimum_density >= 1.0 ||
    !std::isfinite(volume.minimum_volume_fraction) ||
    volume.minimum_volume_fraction < 0.0 ||
    volume.minimum_volume_fraction > 1.0 ||
    !std::isfinite(volume.minimum_design_volume_fraction) ||
    volume.minimum_design_volume_fraction < 0.0 ||
    volume.minimum_design_volume_fraction > 1.0 ||
    configuration.target_volume_fraction + 1.0e-12 <
        volume.minimum_design_volume_fraction)
  {
    PRINT_ERROR("Invalid topology density update data.\n");
    return false;
  }

  // Initial algorithm defaults belong to CoreTopology.
  const double move_limit = configuration.move_limit;
  if (!std::isfinite(move_limit) ||
      move_limit <= 0.0 || move_limit > 1.0)
  {
    PRINT_ERROR("Invalid topology move limit.\n");
    return false;
  }
  if (configuration.objective == TopologyObjective::Displacement)
  {
    return build_displacement_density_update(configuration, design->second, evaluation->second, densities);
  }

  double effective_move_limit = move_limit;
  bool correlation_available = false;
  double correlation = 0.0;

  if (!select_compliance_move_limit(configuration, source_iteration, effective_move_limit, correlation_available, correlation))
  {
    return false;
  }

  if (!build_compliance_oc_proposal(configuration, design->second, evaluation->second, effective_move_limit, densities))
  {
    return false;
  }

  if (correlation_available)
  {
    PRINT_INFO("Topology %d iteration %d: next compliance move limit %.6g; density update correlation %.6g.\n", topology_id, source_iteration, effective_move_limit, correlation);
  }
  else
  {
    PRINT_INFO("Topology %d iteration %d: next compliance move limit %.6g.\n", topology_id, source_iteration, effective_move_limit);
  }

  return true;
}

bool CoreTopology::build_compliance_oc_proposal(const TopologyCase& configuration, const TopologyDensityDesign& design, const TopologyIterationEvaluation& evaluation, double move_limit, std::map<int, double>& densities) const
{
  densities.clear();

  // The caller validates the iteration maps and configuration.
  // This method also permits a smaller move limit for adaptive updates.
  if (!std::isfinite(move_limit) ||
      move_limit <= 0.0 ||
      move_limit > configuration.move_limit)
  {
    PRINT_ERROR("Invalid compliance OC proposal move limit.\n");
    return false;
  }

  const auto& previous = design.element_densities;
  const auto& gradients = evaluation.filtered_density_sensitivities;
  const auto& volume = design.volume;
  const double minimum_density = design.minimum_density;

  constexpr double volume_tolerance = 1.0e-10;

  struct Variable
  {
    int id;
    double density;
    double lower;
    double upper;
    double volume_weight;
    double benefit;
  };

  std::vector<Variable> variables;
  std::map<int, double> fixed;
  double minimum_reachable = volume.minimum_volume_fraction;
  double maximum_benefit = 0.0;

  for (const auto& density : previous)
  {
    const auto preserved = configuration.element_preserved.find(density.first);
    const auto gradient = gradients.find(density.first);
    const auto weight = volume.density_sensitivities.find(density.first);

    if (preserved == configuration.element_preserved.end() ||
        gradient == gradients.end() ||
        weight == volume.density_sensitivities.end() ||
        !std::isfinite(density.second) ||
        density.second < minimum_density || density.second > 1.0 ||
        !std::isfinite(gradient->second) ||
        gradient->second > 0.0 ||
        !std::isfinite(weight->second))
    {
      PRINT_ERROR("Invalid update data for element %d.\n", density.first);
      return false;
    }

    if (preserved->second)
    {
      if (density.second != 1.0 || weight->second != 0.0)
      {
        PRINT_ERROR("Invalid preserved element %d.\n", density.first);
        return false;
      }

      fixed.emplace(density.first, 1.0);
      continue;
    }

    if (weight->second <= 0.0)
    {
      PRINT_ERROR("Invalid volume derivative for element %d.\n", density.first);
      return false;
    }

    const double lower = std::max(minimum_density, density.second - move_limit);
    const double upper = std::min(1.0, density.second + move_limit);
    const double benefit = -gradient->second / weight->second;

    if (!std::isfinite(benefit))
    {
      PRINT_ERROR("Invalid update benefit for element %d.\n", density.first);
      return false;
    }

    variables.push_back({density.first, density.second, lower, upper, weight->second, benefit});
    minimum_reachable += lower * weight->second;
    maximum_benefit = std::max(maximum_benefit, benefit);
  }

  if (!std::isfinite(minimum_reachable))
  {
    PRINT_ERROR("Invalid reachable topology volume.\n");
    return false;
  }

  // Respect the move limit while approaching the final target.
  const double target = std::max(configuration.target_volume_fraction, minimum_reachable);

  // Normalize benefits to reduce dependence on model units.
  for (auto& variable : variables)
  {
    if (maximum_benefit > 0.0)
    {
      variable.benefit /= maximum_benefit;
    }
  }

  const auto make_trial = [&](double multiplier, std::map<int, double>& trial)
  {
    trial = fixed;
    double fraction = volume.minimum_volume_fraction;

    for (const auto& variable : variables)
    {
      double density = variable.lower;

      if (variable.benefit > 0.0)
      {
        if (multiplier == 0.0)
        {
          density = variable.upper;
        }
        else
        {
          density = variable.density * std::sqrt(variable.benefit / multiplier);

          density = std::max(variable.lower, std::min(variable.upper, density));
        }
      }
      trial.emplace(variable.id, density);
      fraction += density * variable.volume_weight;
    }
    return fraction;
  };

  std::map<int, double> candidate;
  double fraction = make_trial(0.0, candidate);

  if (fraction <= target + volume_tolerance)
  {
    densities.swap(candidate);
    return true;
  }

  double lower_multiplier = 0.0;
  double upper_multiplier = 1.0;

  // Find a multiplier yielding a volume-feasible candidate.
  for (int i = 0; i < 100; ++i)
  {
    fraction = make_trial(upper_multiplier, candidate);
    if (fraction <= target + volume_tolerance)
    {
      break;
    }

    upper_multiplier *= 2.0;
  }

  if (!std::isfinite(fraction) || fraction > target + volume_tolerance)
  {
    PRINT_ERROR("Unable to bracket the topology volume constraint.\n");
    return false;
  }

  // Keep the feasible candidate while refining the multiplier.
  std::map<int, double> feasible = candidate;

  for (int i = 0; i < 100; ++i)
  {
    const double multiplier = 0.5 * (lower_multiplier + upper_multiplier);

    fraction = make_trial(multiplier, candidate);

    if (!std::isfinite(fraction))
    {
      PRINT_ERROR("Invalid trial topology volume.\n");
      return false;
    }

    if (fraction > target)
    {
      lower_multiplier = multiplier;
    }
    else
    {
      upper_multiplier = multiplier;
      feasible = candidate;

      if (target - fraction <= volume_tolerance)
      {
        break;
      }
    }
  }

  densities.swap(feasible);
  return true;
}

bool CoreTopology::build_displacement_density_update(const TopologyCase& configuration, const TopologyDensityDesign& design, const TopologyIterationEvaluation& evaluation, std::map<int, double>& densities) const
{
  densities.clear();

  const auto& previous = design.element_densities;
  const auto& gradients = evaluation.filtered_density_sensitivities;
  const auto& volume = design.volume;

  const double move_limit = configuration.move_limit;
  constexpr double volume_tolerance = 1.0e-10;

  struct Variable
  {
    int id;
    double density;
    double lower;
    double upper;
    double volume_weight;
    double direction;
    double trial;
  };

  std::vector<Variable> variables;
  std::map<int, double> fixed;

  double minimum_reachable = volume.minimum_volume_fraction;
  double maximum_direction = 0.0;
  double weight_sum = volume.minimum_volume_fraction;

  for (const auto& density : previous)
  {
    const auto gradient = gradients.find(density.first);
    const auto weight = volume.density_sensitivities.find(density.first);
    const auto preserved = configuration.element_preserved.find(density.first);

    if (gradient == gradients.end() ||
        weight == volume.density_sensitivities.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(density.second) ||
        density.second < design.minimum_density ||
        density.second > 1.0 ||
        !std::isfinite(gradient->second) ||
        !std::isfinite(weight->second))
    {
      PRINT_ERROR("Invalid displacement update data for element %d.\n", density.first);
      return false;
    }

    if (preserved->second)
    {
      if (density.second != 1.0 || weight->second != 0.0 || gradient->second != 0.0)
      {
        PRINT_ERROR("Invalid preserved displacement update element %d.\n", density.first);
        return false;
      }

      fixed.emplace(density.first, 1.0);
      continue;
    }

    if (weight->second <= 0.0)
    {
      PRINT_ERROR("Invalid volume derivative for element %d.\n", density.first);
      return false;
    }

    const double lower = std::max(design.minimum_density, density.second - move_limit);
    const double upper = std::min(1.0, density.second + move_limit);
    const double direction = gradient->second / weight->second;

    if (!std::isfinite(direction))
    {
      PRINT_ERROR("Invalid displacement update direction for element %d.\n", density.first);
      return false;
    }

    variables.push_back({density.first, density.second, lower, upper, weight->second, direction, density.second});

    minimum_reachable += lower * weight->second;
    weight_sum += weight->second;
    maximum_direction = std::max(maximum_direction, std::abs(direction));
  }

  if (!std::isfinite(minimum_reachable) ||
      !std::isfinite(weight_sum) ||
      std::abs(weight_sum - 1.0) > volume_tolerance)
  {
    PRINT_ERROR("Invalid displacement update volume weights.\n");
    return false;
  }

  // Approach the final volume target while respecting the move limits.
  const double target = std::max(configuration.target_volume_fraction, minimum_reachable);

  double upper_multiplier = 0.0;

  for (auto& variable : variables)
  {
    double normalized_direction = 0.0;

    if (maximum_direction > 0.0)
    {
      normalized_direction = variable.direction / maximum_direction;
    }

    variable.trial = variable.density - move_limit * normalized_direction;

    // At this multiplier every free element reaches its lower bound.
    upper_multiplier = std::max(upper_multiplier, variable.trial - variable.lower);
  }

  /*
   * Project the unconstrained trial onto:
   *   lower <= rho <= upper
   *   preserved_fraction + sum(volume_weight * rho) <= target
   *
   * Using the volume-weighted squared distance gives one common
   * multiplier: rho = clamp(trial - multiplier, lower, upper).
   */
  const auto make_trial = [&](double multiplier, std::map<int, double>& trial)
  {
    trial = fixed;
    double fraction = volume.minimum_volume_fraction;

    for (const auto& variable : variables)
    {
      const double density = std::max(variable.lower, std::min(variable.upper, variable.trial - multiplier));

      trial.emplace(variable.id, density);
      fraction += variable.volume_weight * density;
    }

    return fraction;
  };

  std::map<int, double> candidate;
  double fraction = make_trial(0.0, candidate);

  if (!std::isfinite(fraction))
  {
    PRINT_ERROR("Invalid displacement trial volume.\n");
    return false;
  }

  // The volume constraint is an upper bound; unused volume is allowed.
  if (fraction <= target + volume_tolerance)
  {
    densities.swap(candidate);
    return true;
  }

  fraction = make_trial(upper_multiplier, candidate);

  if (!std::isfinite(fraction) || fraction > target + volume_tolerance)
  {
    PRINT_ERROR("Unable to bracket the displacement update volume constraint.\n");
    return false;
  }

  double lower_multiplier = 0.0;
  std::map<int, double> feasible = candidate;

  for (int iteration = 0; iteration < 100; ++iteration)
  {
    const double multiplier = 0.5 * (lower_multiplier + upper_multiplier);

    fraction = make_trial(multiplier, candidate);

    if (!std::isfinite(fraction))
    {
      PRINT_ERROR("Invalid projected displacement update volume.\n");
      return false;
    }

    if (fraction > target)
    {
      lower_multiplier = multiplier;
    }
    else
    {
      upper_multiplier = multiplier;
      feasible = candidate;

      if (target - fraction <= volume_tolerance)
      {
        break;
      }
    }
  }

  densities.swap(feasible);
  return true;
}

bool CoreTopology::evaluate_compliance_oc_residual(int topology_id, int iteration_id)
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || iteration_id < 1 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid compliance OC residual request.\n");
    return false;
  }

  auto& configuration = entry->second;
  const auto design = configuration.iteration_density_designs.find(iteration_id);
  const auto evaluation = configuration.iteration_evaluations.find(iteration_id);

  if (configuration.method != TopologyMethod::Density ||
      configuration.objective != TopologyObjective::Compliance ||
      design == configuration.iteration_density_designs.end() ||
      evaluation == configuration.iteration_evaluations.end() ||
      evaluation->second.objective != TopologyObjective::Compliance ||
      !design->second.volume.available)
  {
    PRINT_ERROR("Missing evaluated compliance iteration for OC residual.\n");
    return false;
  }

  auto& progress = evaluation->second;
  progress.oc_proposal_residual_available = false;
  progress.oc_proposal_residual = 0.0;

  const auto& stored_design = design->second;
  const auto& densities = stored_design.element_densities;
  const auto& gradients = progress.filtered_density_sensitivities;
  const auto& volume = stored_design.volume;

  if (densities.empty() ||
      densities.size() != gradients.size() ||
      densities.size() != volume.density_sensitivities.size() ||
      densities.size() != configuration.element_preserved.size() ||
      !std::isfinite(stored_design.minimum_density) ||
      stored_design.minimum_density <= 0.0 ||
      stored_design.minimum_density >= 1.0 ||
      !std::isfinite(configuration.move_limit) ||
      configuration.move_limit <= 0.0 ||
      configuration.move_limit > 1.0 ||
      !std::isfinite(configuration.target_volume_fraction) ||
      configuration.target_volume_fraction <= 0.0 ||
      configuration.target_volume_fraction > 1.0 ||
      !std::isfinite(volume.minimum_volume_fraction) ||
      volume.minimum_volume_fraction < 0.0 ||
      volume.minimum_volume_fraction > 1.0 ||
      !std::isfinite(volume.minimum_design_volume_fraction) ||
      volume.minimum_design_volume_fraction < 0.0 ||
      volume.minimum_design_volume_fraction > 1.0 ||
      configuration.target_volume_fraction + 1.0e-12 < volume.minimum_design_volume_fraction)
  {
    PRINT_ERROR("Invalid density data for compliance OC residual.\n");
    return false;
  }

  std::map<int, double> proposal;

  // Always use the configured maximum, never the adaptive move limit.
  if (!build_compliance_oc_proposal(configuration, stored_design, progress, configuration.move_limit, proposal))
  {
    return false;
  }

  if (proposal.size() != densities.size())
  {
    PRINT_ERROR("Incomplete ordinary compliance OC proposal.\n");
    return false;
  }

  double residual = 0.0;

  for (const auto& density : densities)
  {
    const auto proposed = proposal.find(density.first);

    if (proposed == proposal.end() ||
        !std::isfinite(proposed->second) ||
        proposed->second < stored_design.minimum_density ||
        proposed->second > 1.0)
    {
      PRINT_ERROR("Invalid ordinary OC proposal for element %d.\n", density.first);
      return false;
    }

    residual = std::max(residual, std::abs(proposed->second - density.second));
  }

  if (!std::isfinite(residual))
  {
    PRINT_ERROR("Invalid ordinary compliance OC residual.\n");
    return false;
  }

  progress.oc_proposal_residual = residual;
  progress.oc_proposal_residual_available = true;
  return true;
}

bool CoreTopology::check_density_convergence(int topology_id,int previous_iteration, int current_iteration, bool& converged)
{
  converged = false;

  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized ||
      previous_iteration < 1 ||
      current_iteration <= previous_iteration ||
      entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology convergence request.\n");
    return false;
  }

  auto& configuration = entry->second;
  const auto previous = configuration.iteration_density_designs.find(previous_iteration);
  const auto current = configuration.iteration_density_designs.find(current_iteration);
  const auto previous_evaluation = configuration.iteration_evaluations.find(previous_iteration);
  const auto current_evaluation = configuration.iteration_evaluations.find(current_iteration);

  const char *objective_name = topology_objective_name(configuration.objective);

  if (configuration.method != TopologyMethod::Density ||
      objective_name == nullptr ||
      previous == configuration.iteration_density_designs.end() ||
      current == configuration.iteration_density_designs.end() ||
      previous_evaluation == configuration.iteration_evaluations.end() ||
      current_evaluation == configuration.iteration_evaluations.end() ||
      previous_evaluation->second.objective != configuration.objective ||
      current_evaluation->second.objective != configuration.objective ||
      !current->second.volume.available)
  {
    PRINT_ERROR("Missing or inconsistent topology convergence data.\n");
    return false;
  }

  const auto& previous_densities = previous->second.element_densities;
  const auto& current_densities = current->second.element_densities;

  if (previous_densities.empty() ||
      previous_densities.size() != current_densities.size())
  {
    PRINT_ERROR("Incomplete topology convergence densities.\n");
    return false;
  }

  double maximum_change = 0.0;

  for (const auto& density : current_densities)
  {
    const auto old = previous_densities.find(density.first);

    if (old == previous_densities.end() ||
        !std::isfinite(density.second) ||
        !std::isfinite(old->second) ||
        density.second < 0.0 || density.second > 1.0 ||
        old->second < 0.0 || old->second > 1.0)
    {
      PRINT_ERROR("Invalid convergence density for element %d.\n",
                  density.first);
      return false;
    }

    maximum_change = std::max(maximum_change, std::abs(density.second - old->second));
  }

  const double previous_value = previous_evaluation->second.value;
  const double current_value = current_evaluation->second.value;
  const double fraction = current->second.volume.volume_fraction;

  if (!std::isfinite(previous_value) ||
      !std::isfinite(current_value) ||
      previous_value < 0.0 || current_value < 0.0 ||
      !std::isfinite(fraction) ||
      fraction < 0.0 || fraction > 1.0 ||
      !std::isfinite(configuration.target_volume_fraction) ||
      configuration.target_volume_fraction <= 0.0 ||
      configuration.target_volume_fraction > 1.0)
  {
    PRINT_ERROR("Invalid topology convergence values.\n");
    return false;
  }

  const double scale = std::max(previous_value, current_value);
  double relative_change = 0.0;

  if (scale > 0.0)
  {
    relative_change = std::abs(current_value / scale - previous_value / scale);
  }

  const double density_tolerance = configuration.density_tolerance;

  // Retain the existing command and persistence field for compatibility.
  const double objective_tolerance = configuration.compliance_tolerance;

  if (!std::isfinite(density_tolerance) ||
      density_tolerance <= 0.0 || density_tolerance > 1.0 ||
      !std::isfinite(objective_tolerance) ||
      objective_tolerance <= 0.0 || objective_tolerance > 1.0)
  {
    PRINT_ERROR("Invalid topology convergence tolerances.\n");
    return false;
  }

  const bool needs_oc_residual = configuration.objective == TopologyObjective::Compliance;
  const auto& current_progress = current_evaluation->second;

  if (needs_oc_residual &&
      current_progress.oc_proposal_residual_available &&
      (!std::isfinite(current_progress.oc_proposal_residual) ||
       current_progress.oc_proposal_residual < 0.0))
  {
    PRINT_ERROR("Invalid ordinary compliance OC residual.\n");
    return false;
  }

  const bool oc_residual_passed = !needs_oc_residual ||
      (current_progress.oc_proposal_residual_available &&
       current_progress.oc_proposal_residual <= density_tolerance);

  constexpr double volume_tolerance = 1.0e-6;

  converged =
      maximum_change <= density_tolerance &&
      relative_change <= objective_tolerance &&
      fraction <= configuration.target_volume_fraction + volume_tolerance &&
      oc_residual_passed;

  auto& progress = current_evaluation->second;
  progress.maximum_density_change = maximum_change;
  progress.relative_objective_change = relative_change;
  progress.converged = converged;
  progress.convergence_checked = true;

  PRINT_INFO("Topology %d iteration %d convergence:\n", topology_id, current_iteration);
  PRINT_INFO("  Maximum density change: %.6g / %.6g (%.2fx limit) [%s]\n", maximum_change, density_tolerance, maximum_change / density_tolerance, maximum_change <= density_tolerance ? "PASS" : "WAIT");
  if (needs_oc_residual)
  {
    if (current_progress.oc_proposal_residual_available)
    {
      PRINT_INFO("  Ordinary OC proposal residual: %.6g / %.6g (%.2fx limit) [%s]\n", current_progress.oc_proposal_residual, density_tolerance, current_progress.oc_proposal_residual / density_tolerance, oc_residual_passed ? "PASS" : "WAIT");
    }
    else
    {
      PRINT_INFO("  Ordinary OC proposal residual: unavailable [WAIT]\n");
    }
  }
  PRINT_INFO("  Relative %s change: %.6g / %.6g (%.2fx limit) [%s]\n", objective_name, relative_change, objective_tolerance, relative_change / objective_tolerance, relative_change <= objective_tolerance ? "PASS" : "WAIT");
  PRINT_INFO("  Volume fraction: %.9g / %.9g [%s]\n", fraction, configuration.target_volume_fraction + volume_tolerance, fraction <= configuration.target_volume_fraction + volume_tolerance ? "PASS" : "WAIT");
  PRINT_INFO("  Converged: %s\n", converged ? "YES" : "NO");

  return true;
}

bool CoreTopology::filter_density_sensitivities(int topology_id,int iteration_id,const std::map<int, double>& element_volumes,const std::map<int, std::array<double, 3>>& element_centers)
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || iteration_id < 1 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology sensitivity filter request.\n");
    return false;
  }

  auto& configuration = entry->second;
  const auto design = configuration.iteration_density_designs.find(iteration_id);
  const auto evaluation = configuration.iteration_evaluations.find(iteration_id);

  if (configuration.method != TopologyMethod::Density ||
      topology_objective_name(configuration.objective) == nullptr ||
      design == configuration.iteration_density_designs.end() ||
      evaluation == configuration.iteration_evaluations.end() ||
      evaluation->second.objective != configuration.objective ||
      !design->second.volume.available)
  {
    PRINT_ERROR("Missing or inconsistent topology sensitivity filter data.\n");
    return false;
  }

  const auto& densities = design->second.element_densities;
  const auto& gradients = evaluation->second.density_sensitivities;

  if (densities.empty() ||
      densities.size() != gradients.size() ||
      densities.size() != element_volumes.size() ||
      densities.size() != element_centers.size() ||
      densities.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology sensitivity filter data.\n");
    return false;
  }

  double maximum_volume = 0.0;

  for (const auto& density : densities)
  {
    const auto gradient = gradients.find(density.first);
    const auto volume = element_volumes.find(density.first);
    const auto center = element_centers.find(density.first);
    const auto preserved = configuration.element_preserved.find(density.first);

    if (gradient == gradients.end() ||
        volume == element_volumes.end() ||
        center == element_centers.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(density.second) ||
        density.second <= 0.0 || density.second > 1.0 ||
        !std::isfinite(gradient->second) ||
        !std::isfinite(volume->second) ||
        volume->second <= 0.0)
    {
      PRINT_ERROR("Invalid filter data for element %d.\n", density.first);
      return false;
    }

    for (double coordinate : center->second)
    {
      if (!std::isfinite(coordinate))
      {
        PRINT_ERROR("Invalid filter center for element %d.\n", density.first);
        return false;
      }
    }

    maximum_volume = std::max(maximum_volume, volume->second);
  }

  // Initial automatic radius default belongs to CoreTopology.
  double radius = configuration.filter_radius;

  if (!std::isfinite(radius) || radius < 0.0)
  {
    PRINT_ERROR("Invalid topology filter radius setting.\n");
    return false;
  }

  if (radius == 0.0)
  {
    radius = 1.5 * std::cbrt(design->second.volume.total_volume / static_cast<double>(densities.size()));
  }

  if (!std::isfinite(radius) || radius <= 0.0)
  {
    PRINT_ERROR("Invalid topology sensitivity filter radius.\n");
    return false;
  }

  // make a hash for all elements, to reduce neighbour search costs
  using Cell = std::array<std::int64_t, 3>;
  struct CellHash
  {
    std::size_t operator()(const Cell& cell) const
    {
      std::size_t seed = 0;

      for (std::int64_t coordinate : cell)
      {
        const std::size_t value =
            std::hash<std::int64_t>{}(coordinate);

        seed ^= value + static_cast<std::size_t>(0x9e3779b9U) +
            (seed << 6) + (seed >> 2);
      }

      return seed;
    }
  };

  std::unordered_map<Cell, std::vector<int>, CellHash> cells;
  std::map<int, Cell> element_cells;

  // Shift the grid origin to the mesh bounds.
  std::array<double, 3> origin = element_centers.begin()->second;

  for (const auto& element : element_centers)
  {
    for (int axis = 0; axis < 3; ++axis)
    {
      origin[axis] = std::min(origin[axis], element.second[axis]);
    }
  }

  // Leave ample room for neighboring-cell offsets.
  const double maximum_cell_coordinate = static_cast<double>(
      (std::numeric_limits<std::int64_t>::max)() / 2);

  for (const auto& element : densities)
  {
    if (configuration.element_preserved.find(element.first)->second)
    {
      continue;
    }

    const auto& center = element_centers.find(element.first)->second;
    Cell cell{};

    for (int axis = 0; axis < 3; ++axis)
    {
      const double coordinate =
          std::floor((center[axis] - origin[axis]) / radius);

      if (!std::isfinite(coordinate) ||
          coordinate < 0.0 ||
          coordinate > maximum_cell_coordinate)
      {
        PRINT_ERROR("Invalid spatial cell for element %d.\n",
                    element.first);
        return false;
      }

      cell[axis] = static_cast<std::int64_t>(coordinate);
    }

    cells[cell].push_back(element.first);
    element_cells.emplace(element.first, cell);
  }


  std::map<int, double> filtered;

  for (const auto& element : densities)
  {
    if (configuration.element_preserved.find(element.first)->second)
    {
      filtered.emplace(element.first, 0.0);
      continue;
    }

    const auto& center = element_centers.find(element.first)->second;
    double numerator = 0.0;
    double denominator = 0.0;

    const Cell& own_cell = element_cells.find(element.first)->second;

    for (int dx = -1; dx <= 1; ++dx)
    {
      for (int dy = -1; dy <= 1; ++dy)
      {
        for (int dz = -1; dz <= 1; ++dz)
        {
          const Cell neighbor_cell{
              own_cell[0] + dx,
              own_cell[1] + dy,
              own_cell[2] + dz};

          const auto bucket = cells.find(neighbor_cell);

          if (bucket == cells.end())
          {
            continue;
          }

          for (int neighbor_id : bucket->second)
          {
            const auto& other =
                element_centers.find(neighbor_id)->second;

            const double distance = std::hypot(
                std::hypot(
                    center[0] - other[0],
                    center[1] - other[1]),
                center[2] - other[2]);

            if (!std::isfinite(distance))
            {
              PRINT_ERROR("Invalid distance in topology filter.\n");
              return false;
            }

            if (distance >= radius)
            {
              continue;
            }

            const double neighbor_volume =
                element_volumes.find(neighbor_id)->second;

            const double weight =
                (1.0 - distance / radius) *
                (neighbor_volume / maximum_volume);

            const double neighbor_density =
                densities.find(neighbor_id)->second;

            const double sensitivity_per_volume =
                gradients.find(neighbor_id)->second / neighbor_volume;

            numerator +=
                weight * neighbor_density * sensitivity_per_volume;

            denominator += weight;
          }
        }
      }
    }

    if (!std::isfinite(numerator) ||
        !std::isfinite(denominator) ||
        denominator <= 0.0)
    {
      PRINT_ERROR("Invalid filter accumulation for element %d.\n", element.first);
      return false;
    }

    const double element_volume = element_volumes.find(element.first)->second;
    const double sensitivity =(numerator / denominator) * element_volume / element.second;

    if (!std::isfinite(sensitivity))
    {
      PRINT_ERROR("Invalid filtered sensitivity for element %d.\n",  element.first);
      return false;
    }
    filtered.emplace(element.first, sensitivity);
  }

  evaluation->second.filtered_density_sensitivities.swap(filtered);
  evaluation->second.sensitivity_filter_radius = radius;
  evaluation->second.oc_proposal_residual_available = false;
  evaluation->second.oc_proposal_residual = 0.0;

  return true;
}

std::vector<std::vector<std::string>> CoreTopology::get_tree_data() const
{
  std::vector<std::vector<std::string>> rows;

  for (const auto& entry : topology_cases)
  {
    rows.push_back({std::to_string(entry.first), entry.second.name});
  }

  return rows;
}

bool CoreTopology::reset_preserved_sets(int topology_id)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;

  if (configuration.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is running; preservation cannot be reset.\n",
      topology_id);
    return false;
  }

  configuration.added_preserved_sets.clear();
  configuration.removed_preserved_sets.clear();

  return true;
}

bool CoreTopology::change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding)
{
  CalculiXMeshInterface ccx_mesh_iface;

  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (entry->second.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is running.\n", topology_id);
    return false;
  }

  if (set_type < 1 || set_type > 3)
  {
    PRINT_ERROR("Invalid topology preserved set type.\n");
    return false;
  }

  // Validate the complete list before changing configuration.
  for (int id : ids)
  {
    bool exists = false;

    if (id > 0)
    {
      if (set_type == 1)
      {
        exists = ccx_mesh_iface.check_block_exists(id);
      }
      else if (set_type == 2)
      {
        exists = ccx_mesh_iface.check_nodeset_exists(id);
      }
      else
        exists = ccx_mesh_iface.check_sideset_exists(id);
    }

    if (!exists)
    {
      PRINT_ERROR("Preserved set type %d, ID %d does not exist.\n", set_type, id);
      return false;
    }
  }

  auto& configuration = entry->second;

  for (int id : ids)
  {
    if (adding)
    {
      configuration.removed_preserved_sets[set_type].erase(id);
      configuration.added_preserved_sets[set_type].insert(id);
    }
    else
    {
      configuration.added_preserved_sets[set_type].erase(id);
      configuration.removed_preserved_sets[set_type].insert(id);
    }
  }

  return true;
}

bool CoreTopology::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return change_preserved_sets(topology_id, set_type, ids, true);
}

bool CoreTopology::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return change_preserved_sets(topology_id, set_type, ids, false);
}
