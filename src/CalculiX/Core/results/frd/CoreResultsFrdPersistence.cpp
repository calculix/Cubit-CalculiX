#include "Core/results/frd/CoreResultsFrdPersistence.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "HDF5Tool.hpp"

#include <cstddef>
#include <string>

void CoreResultsFrdPersistence::read_base_data(HDF5Tool& file, CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

  file.read_dataset_string_rank_2("header", group, results.header);
  file.read_dataset_string_rank_2("materials", group, results.materials);
  file.read_dataset_int_rank_2("nodes", group, results.nodes);
  file.read_dataset_double_rank_2("nodes_coords", group, results.nodes_coords);
  file.read_dataset_int_rank_2("elements", group, results.elements);
  file.read_dataset_int_rank_2("elements_connectivity", group, results.elements_connectivity);
  file.read_dataset_int_rank_2("result_blocks", group, results.result_blocks);
  file.read_dataset_double_rank_1("total_times", group, results.total_times);
}

void CoreResultsFrdPersistence::write_base_data(HDF5Tool& file, const CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

  file.createGroup(group);
  file.write_dataset_string_rank_2("header", group, results.header);
  file.write_dataset_string_rank_2("materials", group, results.materials);
  file.write_dataset_int_rank_2("nodes", group, results.nodes);
  file.write_dataset_double_rank_2("nodes_coords", group, results.nodes_coords);
  file.write_dataset_int_rank_2("elements", group, results.elements);
  file.write_dataset_int_rank_2("elements_connectivity", group, results.elements_connectivity);
  file.write_dataset_int_rank_2("result_blocks", group, results.result_blocks);
  file.write_dataset_double_rank_1("total_times", group, results.total_times);
}

void CoreResultsFrdPersistence::read_result_blocks(HDF5Tool& file, CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";
  const std::string components = group + "result_block_components/";

  if (file.nameExists(components))
  {
    int index = 0;

    while (file.nameExists(components + std::to_string(index) + "/"))
    {
      results.result_block_components.emplace_back();
      file.read_dataset_string_rank_1(std::to_string(index), components, results.result_block_components[index]);
      ++index;
    }
  }

  file.read_dataset_string_rank_1("result_block_type", group, results.result_block_type);

  const std::string values = group + "result_block_data/";

  if (file.nameExists(values))
  {
    int index = 0;

    while (file.nameExists(values + std::to_string(index) + "/"))
    {
      results.result_block_data.emplace_back();
      file.read_dataset_double_rank_2(std::to_string(index), values, results.result_block_data[index]);
      ++index;
    }
  }

  const std::string nodes = group + "result_block_node_data/";

  if (file.nameExists(nodes))
  {
    int index = 0;

    while (file.nameExists(nodes + std::to_string(index) + "/"))
    {
      results.result_block_node_data.emplace_back();
      file.read_dataset_int_rank_2(std::to_string(index), nodes, results.result_block_node_data[index]);
      ++index;
    }
  }
}

void CoreResultsFrdPersistence::write_result_blocks(HDF5Tool& file, const CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

  if (!results.result_block_components.empty())
  {
    const std::string subgroup = group + "result_block_components/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_block_components.size(); index++)
    {
      file.write_dataset_string_rank_1(std::to_string(index), subgroup, results.result_block_components[index]);
    }
  }

  file.write_dataset_string_rank_1("result_block_type", group, results.result_block_type);

  if (!results.result_block_data.empty())
  {
    const std::string subgroup = group + "result_block_data/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_block_data.size(); index++)
    {
      file.write_dataset_double_rank_2(std::to_string(index), subgroup, results.result_block_data[index]);
    }
  }

  if (!results.result_block_node_data.empty())
  {
    const std::string subgroup = group + "result_block_node_data/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_block_node_data.size(); index++)
    {
      file.write_dataset_int_rank_2(std::to_string(index), subgroup, results.result_block_node_data[index]);
    }
  }
}

void CoreResultsFrdPersistence::read_sorted_indices(HDF5Tool& file, CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

  file.read_dataset_int_rank_1("sorted_node_ids", group, results.sorted_node_ids);
  file.read_dataset_int_rank_1("sorted_node_data_ids", group, results.sorted_node_data_ids);

  const std::string node_ids = group + "sorted_result_node_ids/";

  if (file.nameExists(node_ids))
  {
    int index = 0;

    while (file.nameExists(node_ids + std::to_string(index) + "/"))
    {
      results.sorted_result_node_ids.emplace_back();
      file.read_dataset_int_rank_1(std::to_string(index), node_ids, results.sorted_result_node_ids[index]);
      ++index;
    }
  }

  const std::string data_ids = group + "sorted_result_node_data_ids/";

  if (file.nameExists(data_ids))
  {
    int index = 0;

    while (file.nameExists(data_ids + std::to_string(index) + "/"))
    {
      results.sorted_result_node_data_ids.emplace_back();
      file.read_dataset_int_rank_1(std::to_string(index), data_ids, results.sorted_result_node_data_ids[index]);
      ++index;
    }
  }
}

void CoreResultsFrdPersistence::write_sorted_indices(HDF5Tool& file, const CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

  file.write_dataset_int_rank_1("sorted_node_ids", group, results.sorted_node_ids);
  file.write_dataset_int_rank_1("sorted_node_data_ids", group, results.sorted_node_data_ids);

  if (!results.sorted_result_node_ids.empty())
  {
    const std::string subgroup = group + "sorted_result_node_ids/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.sorted_result_node_ids.size(); index++)
    {
      file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_node_ids[index]);
    }
  }

  if (!results.sorted_result_node_data_ids.empty())
  {
    const std::string subgroup = group + "sorted_result_node_data_ids/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.sorted_result_node_data_ids.size(); index++)
    {
      file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_node_data_ids[index]);
    }
  }
}

void CoreResultsFrdPersistence::read(HDF5Tool& file, CoreResultsFrd& results)
{
  const std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(results.job_id) + "/";

  if (!file.nameExists(group))
  {
    return;
  }

  read_base_data(file, results);
  read_result_blocks(file, results);
  read_sorted_indices(file, results);
}

void CoreResultsFrdPersistence::write(HDF5Tool& file, const CoreResultsFrd& results)
{
  write_base_data(file, results);
  write_result_blocks(file, results);
  write_sorted_indices(file, results);
}
