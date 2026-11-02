#include "Core/results/frd/CoreResultsFrdPersistence.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "HDF5Tool.hpp"

#include <cstddef>
#include <string>

#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <algorithm>
#include <exception>
#include <stdexcept>

namespace
{
  struct SavedFrdData
  {
    int job_id = -1;
    decltype(CoreResultsFrd::elements) elements;
    decltype(CoreResultsFrd::elements_connectivity) elements_connectivity;
    decltype(CoreResultsFrd::header) header;
    decltype(CoreResultsFrd::materials) materials;
    decltype(CoreResultsFrd::nodes) nodes;
    decltype(CoreResultsFrd::nodes_coords) nodes_coords;
    decltype(CoreResultsFrd::result_block_components) result_block_components;
    decltype(CoreResultsFrd::result_block_data) result_block_data;
    decltype(CoreResultsFrd::result_block_node_data) result_block_node_data;
    decltype(CoreResultsFrd::result_block_type) result_block_type;
    decltype(CoreResultsFrd::result_blocks) result_blocks;
    decltype(CoreResultsFrd::sorted_node_data_ids) sorted_node_data_ids;
    decltype(CoreResultsFrd::sorted_node_ids) sorted_node_ids;
    decltype(CoreResultsFrd::sorted_result_node_data_ids) sorted_result_node_data_ids;
    decltype(CoreResultsFrd::sorted_result_node_ids) sorted_result_node_ids;
    decltype(CoreResultsFrd::total_times) total_times;
  };

  std::vector<int> savedIndices(HDF5Tool& file, const std::string& path)
  {
    std::vector<int> indices;
    if (!file.nameExists(path))
    {
      return indices;
    }
    auto group = file.file->openGroup(path);
    for (hsize_t position = 0; position < group.getNumObjs(); ++position)
    {
      const std::string name = group.getObjnameByIdx(position);
      int index = 0;
      if (!PersistenceTable::id(name, index) || index < 0 || index > 1000000 || group.getObjTypeByIdx(position) != H5G_DATASET)
      {
        throw std::runtime_error("Invalid indexed result dataset: " + path + name);
      }
      indices.push_back(index);
    }
    std::sort(indices.begin(), indices.end());
    return indices;
  }

  bool inRange(int index, std::size_t size)
  {
    return index >= 0 && static_cast<std::size_t>(index) < size;
  }

  bool read_base_data(HDF5Tool& file, SavedFrdData& results)
  {
    try
    {
      const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "header") && !file.read_dataset_string_rank_2("header", group, results.header))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "materials") && !file.read_dataset_string_rank_2("materials", group, results.materials))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "nodes") && !file.read_dataset_int_rank_2("nodes", group, results.nodes))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "nodes_coords") && !file.read_dataset_double_rank_2("nodes_coords", group, results.nodes_coords))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "elements") && !file.read_dataset_int_rank_2("elements", group, results.elements))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "elements_connectivity") && !file.read_dataset_int_rank_2("elements_connectivity", group, results.elements_connectivity))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "result_blocks") && !file.read_dataset_int_rank_2("result_blocks", group, results.result_blocks))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "total_times") && !file.read_dataset_double_rank_1("total_times", group, results.total_times))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Frd read_base_data: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Frd read_base_data: %s\n", error.what());
      return false;
    }
  }

  bool write_base_data(HDF5Tool& file, const CoreResultsFrd& results)
  {
    try
    {
      const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

      if (!file.ensureGroup(group))
      {
        PRINT_ERROR("Cannot save CalculiX Frd result group for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_string_rank_2("header", group, results.header))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_string_rank_2("materials", group, results.materials))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_int_rank_2("nodes", group, results.nodes))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_double_rank_2("nodes_coords", group, results.nodes_coords))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_int_rank_2("elements", group, results.elements))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_int_rank_2("elements_connectivity", group, results.elements_connectivity))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_int_rank_2("result_blocks", group, results.result_blocks))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_double_rank_1("total_times", group, results.total_times))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_base_data for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Frd write_base_data: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Frd write_base_data: %s\n", error.what());
      return false;
    }
  }

  bool read_result_blocks(HDF5Tool& file, SavedFrdData& results)
  {
    try
    {
      const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";
      const std::string components = group + "result_block_components/";

      if (file.nameExists(components))
      {
        for (const int index : savedIndices(file, components))
        {
          results.result_block_components.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(components) && file.nameExists(components + std::string("/") + std::to_string(index)) && !file.read_dataset_string_rank_1(std::to_string(index), components, results.result_block_components[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Frd read_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "result_block_type") && !file.read_dataset_string_rank_1("result_block_type", group, results.result_block_type))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      const std::string values = group + "result_block_data/";

      if (file.nameExists(values))
      {
        for (const int index : savedIndices(file, values))
        {
          results.result_block_data.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(values) && file.nameExists(values + std::string("/") + std::to_string(index)) && !file.read_dataset_double_rank_2(std::to_string(index), values, results.result_block_data[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Frd read_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      const std::string nodes = group + "result_block_node_data/";

      if (file.nameExists(nodes))
      {
        for (const int index : savedIndices(file, nodes))
        {
          results.result_block_node_data.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(nodes) && file.nameExists(nodes + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_2(std::to_string(index), nodes, results.result_block_node_data[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Frd read_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Frd read_result_blocks: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Frd read_result_blocks: %s\n", error.what());
      return false;
    }
  }

  bool write_result_blocks(HDF5Tool& file, const CoreResultsFrd& results)
  {
    try
    {
      const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

      if (!results.result_block_components.empty())
      {
        const std::string subgroup = group + "result_block_components/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Frd result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_block_components.size(); index++)
        {
          if (!file.write_dataset_string_rank_1(std::to_string(index), subgroup, results.result_block_components[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Frd write_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!file.write_dataset_string_rank_1("result_block_type", group, results.result_block_type))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      if (!results.result_block_data.empty())
      {
        const std::string subgroup = group + "result_block_data/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Frd result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_block_data.size(); index++)
        {
          if (!file.write_dataset_double_rank_2(std::to_string(index), subgroup, results.result_block_data[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Frd write_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!results.result_block_node_data.empty())
      {
        const std::string subgroup = group + "result_block_node_data/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Frd result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_block_node_data.size(); index++)
        {
          if (!file.write_dataset_int_rank_2(std::to_string(index), subgroup, results.result_block_node_data[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Frd write_result_blocks for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Frd write_result_blocks: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Frd write_result_blocks: %s\n", error.what());
      return false;
    }
  }

  bool read_sorted_indices(HDF5Tool& file, SavedFrdData& results)
  {
    try
    {
      const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "sorted_node_ids") && !file.read_dataset_int_rank_1("sorted_node_ids", group, results.sorted_node_ids))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "sorted_node_data_ids") && !file.read_dataset_int_rank_1("sorted_node_data_ids", group, results.sorted_node_data_ids))
      {
        PRINT_ERROR("Cannot read CalculiX Frd read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      const std::string node_ids = group + "sorted_result_node_ids/";

      if (file.nameExists(node_ids))
      {
        for (const int index : savedIndices(file, node_ids))
        {
          results.sorted_result_node_ids.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(node_ids) && file.nameExists(node_ids + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_1(std::to_string(index), node_ids, results.sorted_result_node_ids[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Frd read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      const std::string data_ids = group + "sorted_result_node_data_ids/";

      if (file.nameExists(data_ids))
      {
        for (const int index : savedIndices(file, data_ids))
        {
          results.sorted_result_node_data_ids.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(data_ids) && file.nameExists(data_ids + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_1(std::to_string(index), data_ids, results.sorted_result_node_data_ids[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Frd read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Frd read_sorted_indices: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Frd read_sorted_indices: %s\n", error.what());
      return false;
    }
  }

  bool write_sorted_indices(HDF5Tool& file, const CoreResultsFrd& results)
  {
    try
    {
      const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

      if (!file.write_dataset_int_rank_1("sorted_node_ids", group, results.sorted_node_ids))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_int_rank_1("sorted_node_data_ids", group, results.sorted_node_data_ids))
      {
        PRINT_ERROR("Cannot save CalculiX Frd write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      if (!results.sorted_result_node_ids.empty())
      {
        const std::string subgroup = group + "sorted_result_node_ids/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Frd result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.sorted_result_node_ids.size(); index++)
        {
          if (!file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_node_ids[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Frd write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!results.sorted_result_node_data_ids.empty())
      {
        const std::string subgroup = group + "sorted_result_node_data_ids/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Frd result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.sorted_result_node_data_ids.size(); index++)
        {
          if (!file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_node_data_ids[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Frd write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Frd write_sorted_indices: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Frd write_sorted_indices: %s\n", error.what());
      return false;
    }
  }

  template<class Data>
  bool validFrdData(const Data& results)
  {
    if (!PersistenceTable::rows(results.nodes, 2) || !PersistenceTable::rows(results.nodes_coords, 3) || !PersistenceTable::rows(results.elements, 4) || !PersistenceTable::rows(results.result_blocks, 7))
    {
      return false;
    }
    for (const auto& node : results.nodes)
    {
      if (!inRange(node[1], results.nodes_coords.size()))
      {
        return false;
      }
    }
    for (const auto& element : results.elements)
    {
      if (!inRange(element[2], results.elements_connectivity.size()))
      {
        return false;
      }
    }
    for (const auto& block : results.result_blocks)
    {
      if (!inRange(block[4], results.total_times.size()) || !inRange(block[5], results.result_block_type.size()) || !inRange(block[6], results.result_block_data.size()) || !inRange(block[6], results.result_block_components.size()) || !inRange(block[6], results.result_block_node_data.size()))
      {
        return false;
      }
      const auto& values = results.result_block_data[block[6]];
      if (!PersistenceTable::rows(values, results.result_block_components[block[6]].size()))
      {
        return false;
      }
      for (const auto& node : results.result_block_node_data[block[6]])
      {
        if (node.size() < 2 || !inRange(node[1], values.size()))
        {
          return false;
        }
      }
    }
    if (results.sorted_node_ids.size() != results.sorted_node_data_ids.size() || results.sorted_result_node_ids.size() != results.sorted_result_node_data_ids.size() || !std::is_sorted(results.sorted_node_ids.begin(), results.sorted_node_ids.end()))
    {
      return false;
    }
    for (const int index : results.sorted_node_data_ids)
    {
      if (!inRange(index, results.nodes.size()))
      {
        return false;
      }
    }
    for (std::size_t block = 0; block < results.sorted_result_node_ids.size(); ++block)
    {
      const auto& ids = results.sorted_result_node_ids[block];
      const auto& indices = results.sorted_result_node_data_ids[block];
      if (ids.size() != indices.size() || !std::is_sorted(ids.begin(), ids.end()) || block >= results.result_block_data.size())
      {
        return false;
      }
      for (const int index : indices)
      {
        if (!inRange(index, results.result_block_data[block].size()))
        {
          return false;
        }
      }
    }
    return true;
  }
}

bool CoreResultsFrdPersistence::read(HDF5Tool& file, CoreResultsFrd& results)
{
  try
  {
    const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id);
    if (!file.nameExists(group))
    {
      return true;
    }
    SavedFrdData saved;
    saved.job_id = results.job_id;
    if (!read_base_data(file, saved))
    {
      return false;
    }
    if (!read_result_blocks(file, saved))
    {
      return false;
    }
    if (!read_sorted_indices(file, saved))
    {
      return false;
    }
    if (!validFrdData(saved))
    {
      PRINT_ERROR("Cannot read CalculiX Frd results for job %d: invalid data shape or result reference.\n", results.job_id);
      return false;
    }
    results.elements.swap(saved.elements);
    results.elements_connectivity.swap(saved.elements_connectivity);
    results.header.swap(saved.header);
    results.materials.swap(saved.materials);
    results.nodes.swap(saved.nodes);
    results.nodes_coords.swap(saved.nodes_coords);
    results.result_block_components.swap(saved.result_block_components);
    results.result_block_data.swap(saved.result_block_data);
    results.result_block_node_data.swap(saved.result_block_node_data);
    results.result_block_type.swap(saved.result_block_type);
    results.result_blocks.swap(saved.result_blocks);
    results.sorted_node_data_ids.swap(saved.sorted_node_data_ids);
    results.sorted_node_ids.swap(saved.sorted_node_ids);
    results.sorted_result_node_data_ids.swap(saved.sorted_result_node_data_ids);
    results.sorted_result_node_ids.swap(saved.sorted_result_node_ids);
    results.total_times.swap(saved.total_times);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Frd results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Frd results: %s\n", error.what());
    return false;
  }
}

bool CoreResultsFrdPersistence::write(HDF5Tool& file, const CoreResultsFrd& results)
{
  try
  {
    if (!validFrdData(results))
    {
      PRINT_ERROR("Cannot save CalculiX Frd results for job %d: invalid data shape or result reference.\n", results.job_id);
      return false;
    }
    if (!write_base_data(file, results))
    {
      return false;
    }
    if (!write_result_blocks(file, results))
    {
      return false;
    }
    if (!write_sorted_indices(file, results))
    {
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Frd results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Frd results: %s\n", error.what());
    return false;
  }
}
