#ifndef TOPOLOGYHDF5_HPP
#define TOPOLOGYHDF5_HPP

#include "HDF5Tool.hpp"
#include "CoreTopologyStructs.hpp"
#include "CoreMaterialsStructs.hpp"

#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace TopologyHdf5Detail
{
template<class T>
struct IsMap : std::false_type {};

template<class V>
struct IsMap<std::map<int, V>> : std::true_type {};

template<class T>
struct IsVector : std::false_type {};

template<class V>
struct IsVector<std::vector<V>> : std::true_type {};
}

// IDs remain integers. Large element maps use paired ID/value arrays.
// Only data structs are serialized; ownership stays in the respective cores.
template<bool Reading>
class TopologyHdf5
{
public:
  TopologyHdf5(HDF5Tool& file, std::string group)
      : file(file), group(std::move(group))
  {}

  template<class T>
  bool value(const std::string& name, T& data)
  {
    using U = std::remove_cv_t<T>;

    if constexpr (std::is_arithmetic_v<U> || std::is_enum_v<U> ||
                  std::is_same_v<U, std::string>)
    {
      return scalar(name, data);
    }
    else
    {
      const std::string path = group + "/" + name;
      if constexpr (Reading)
      {
        if (!file.nameExists(path))
        {
          return false;
        }
      }
      else
      {
        if (!file.createGroup(path))
        {
          return false;
        }
      }

      TopologyHdf5 child(file, path);

      if constexpr (TopologyHdf5Detail::IsVector<U>::value)
      {
        int count = 0;
        if constexpr (!Reading)
        {
          if (data.size() >
              static_cast<size_t>((std::numeric_limits<int>::max)()))
          {
            return false;
          }
          count = static_cast<int>(data.size());
        }

        if (!child.scalar("count", count) || count < 0)
        {
          return false;
        }

        if constexpr (Reading)
        {
          U loaded;
          if (count > 0 && !child.vector_data("values", loaded))
          {
            return false;
          }
          if (loaded.size() != static_cast<size_t>(count))
          {
            return false;
          }
          data = std::move(loaded);
          return true;
        }
        else
        {
          return count == 0 || child.vector_data("values", data);
        }
      }
      else if constexpr (std::is_same_v<U, std::set<int>>)
      {
        std::vector<int> ids;
        if constexpr (!Reading)
        {
          ids.assign(data.begin(), data.end());
        }
        if (!child.value("ids", ids))
        {
          return false;
        }
        if constexpr (Reading)
        {
          U loaded(ids.begin(), ids.end());
          if (loaded.size() != ids.size())
          {
            return false;
          }
          data = std::move(loaded);
        }
        return true;
      }
      else if constexpr (TopologyHdf5Detail::IsMap<U>::value)
      {
        using V = typename U::mapped_type;
        std::vector<int> keys;
        if constexpr (!Reading)
        {
          for (const auto& entry : data)
          {
            keys.push_back(entry.first);
          }
        }
        if (!child.value("keys", keys))
        {
          return false;
        }

        std::set<int> unique_keys;
        for (const int key : keys)
        {
          if (key < 0 || !unique_keys.insert(key).second)
          {
            return false;
          }
        }

        if constexpr (std::is_arithmetic_v<V>)
        {
          using Stored = std::conditional_t<
              std::is_same_v<V, double>, double, int>;
          std::vector<Stored> values;
          if constexpr (!Reading)
          {
            for (const auto& entry : data)
            {
              values.push_back(static_cast<Stored>(entry.second));
            }
          }
          if (!child.value("values", values) || values.size() != keys.size())
          {
            return false;
          }
          if constexpr (Reading)
          {
            U loaded;
            for (size_t i = 0; i < keys.size(); ++i)
            {
              if constexpr (std::is_same_v<V, bool>)
              {
                if (values[i] != 0 && values[i] != 1)
                {
                  return false;
                }
              }
              loaded.emplace(keys[i], static_cast<V>(values[i]));
            }
            data = std::move(loaded);
          }
        }
        else
        {
          if constexpr (Reading)
          {
            U loaded;
            for (const int key : keys)
            {
              V entry{};
              if (!child.value("Item_" + std::to_string(key), entry))
              {
                return false;
              }
              loaded.emplace(key, std::move(entry));
            }
            data = std::move(loaded);
          }
          else
          {
            for (const auto& entry : data)
            {
              if (!child.value("Item_" + std::to_string(entry.first),
                               entry.second))
              {
                return false;
              }
            }
          }
        }
        return true;
      }
      else
      {
        return child.record(data);
      }
    }
  }

private:
  HDF5Tool& file;
  std::string group;

  template<class Vector>
  bool vector_data(const std::string& name, Vector& data)
  {
    using U = std::remove_cv_t<Vector>;
    using V = typename U::value_type;
    bool success = false;

    if constexpr (std::is_same_v<V, int>)
    {
      if constexpr (Reading)
      {
        success = file.read_dataset_int_rank_1(name, group, data);
      }
      else
      {
        success = file.write_dataset_int_rank_1(name, group, data);
      }
    }
    else if constexpr (std::is_same_v<V, double>)
    {
      if constexpr (Reading)
      {
        success = file.read_dataset_double_rank_1(name, group, data);
      }
      else
      {
        success = file.write_dataset_double_rank_1(name, group, data);
      }
    }
    else
    {
      static_assert(std::is_same_v<V, std::string>);
      if constexpr (Reading)
      {
        success = file.read_dataset_string_rank_1(name, group, data);
      }
      else
      {
        success = file.write_dataset_string_rank_1(name, group, data);
      }
    }

    if constexpr (Reading && std::is_same_v<V, double>)
    {
      for (const double number : data)
      {
        if (!std::isfinite(number))
        {
          return false;
        }
      }
    }
    return success;
  }

  template<class T>
  bool scalar(const std::string& name, T& data)
  {
    using U = std::remove_cv_t<T>;
    using Stored = std::conditional_t<
        std::is_enum_v<U> || std::is_same_v<U, bool>, int, U>;
    std::vector<Stored> values;
    if constexpr (!Reading)
    {
      values.push_back(static_cast<Stored>(data));
    }
    if (!vector_data(name, values) || values.size() != 1)
    {
      return false;
    }

    if constexpr (Reading)
    {
      if constexpr (std::is_same_v<U, bool>)
      {
        if (values[0] != 0 && values[0] != 1)
        {
          return false;
        }
      }
      else if constexpr (std::is_enum_v<U>)
      {
        int maximum = 0;
        if constexpr (std::is_same_v<U, TopologyMethod>)
        {
          maximum = static_cast<int>(TopologyMethod::LevelSet);
        }
        else if constexpr (std::is_same_v<U, TopologyObjective>)
        {
          maximum = static_cast<int>(TopologyObjective::Displacement);
        }
        else
        {
          static_assert(std::is_same_v<U, TopologyRunState>);
          maximum = static_cast<int>(TopologyRunState::Killed);
        }
        if (values[0] < 0 || values[0] > maximum)
        {
          return false;
        }
      }
      data = static_cast<U>(values[0]);
    }
    return true;
  }

  template<class T>
  bool record(T& data)
  {
    using U = std::remove_cv_t<T>;
    if constexpr (std::is_same_v<U, TopologyCase>)
    {
      if constexpr (Reading)
      {
        if (file.nameExists(group + "/adjoint_jobs"))
        {
          if (!value("adjoint_jobs", data.iteration_adjoint_jobs))
          {
            return false;
          }
        }
        else
        {
          data.iteration_adjoint_jobs.clear();
        }
      }
      else
      {
        if (!value("adjoint_jobs", data.iteration_adjoint_jobs))
        {
          return false;
        }
      }

      // Configuration is stored by CoreTopology::save_configuration().
      return value("preserved", data.element_preserved) &&
          value("selected_iteration", data.selected_iteration) &&
          value("run_state", data.run_state) &&
          value("jobs", data.iteration_jobs) &&
          value("element_energies", data.iteration_element_energies) &&
          value("total_energies", data.iteration_total_energies) &&
          value("evaluations", data.iteration_evaluations) &&
          value("designs", data.iteration_density_designs);
    }
    else if constexpr (std::is_same_v<U, TopologyIterationEvaluation>)
    {
      // Older evaluations have no displacement measurement.
      if (!Reading || file.nameExists(group + "/displacement_available"))
      {
        if (!value("displacement_available", data.displacement_available) || !value("displacement_value", data.displacement_value) || !value("displacement_node", data.displacement_node) || !value("displacement_nodeset", data.displacement_nodeset) || !value("displacement_component", data.displacement_component))
        {
          return false;
        }
      }

      if (!std::isfinite(data.displacement_value) || data.displacement_value < 0.0 || data.displacement_component < 0 || data.displacement_component > 3 || (data.displacement_available && (data.displacement_node <= 0 || data.displacement_nodeset <= 0)))
      {
        return false;
      }

      return value("objective", data.objective) &&
          value("value", data.value) &&
          value("convergence_checked", data.convergence_checked) &&
          value("maximum_density_change", data.maximum_density_change) &&
          value("relative_objective_change", data.relative_objective_change) &&
          value("converged", data.converged) &&
          value("stiffness_sensitivities", data.stiffness_sensitivities) &&
          value("density_sensitivities", data.density_sensitivities) &&
          value("filtered_sensitivities", data.filtered_density_sensitivities) &&
          value("filter_radius", data.sensitivity_filter_radius);
    }
    else if constexpr (std::is_same_v<U, TopologyDensityDesign>)
    {
      return value("penalization", data.penalization) &&
          value("void_ratio", data.void_ratio) &&
          value("minimum_density", data.minimum_density) &&
          value("densities", data.element_densities) &&
          value("volume", data.volume);
    }
    else if constexpr (std::is_same_v<U, TopologyVolumeEvaluation>)
    {
      return value("available", data.available) &&
          value("total_volume", data.total_volume) &&
          value("solid_volume", data.solid_volume) &&
          value("preserved_volume", data.preserved_volume) &&
          value("volume_fraction", data.volume_fraction) &&
          value("minimum_volume_fraction", data.minimum_volume_fraction) &&
          value("minimum_design_volume_fraction", data.minimum_design_volume_fraction) &&
          value("sensitivities", data.density_sensitivities);
    }
    else if constexpr (std::is_same_v<U, TopologyMaterialData>)
    {
      return value("base_materials", data.base_materials) &&
          value("element_materials", data.element_base_material) &&
          value("factors", data.iteration_factors);
    }
    else
    {
      static_assert(std::is_same_v<U, TopologyElasticMaterial>);
      const bool success = value("youngs_modulus", data.youngs_modulus) &&
          value("poissons_ratio", data.poissons_ratio);
      if constexpr (Reading)
      {
        return success && data.youngs_modulus > 0.0 &&
            data.poissons_ratio > -1.0 && data.poissons_ratio < 0.5;
      }
      return success;
    }
  }
};

#endif
