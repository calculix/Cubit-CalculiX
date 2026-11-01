#include "Core/results/dat/CoreResultsDatPersistence.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"

#include <cstddef>
#include <string>
#include <vector>

void CoreResultsDatPersistence::read_block_metadata(HDF5Tool& file, CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";

  file.read_dataset_int_rank_2("result_blocks", group, results.result_blocks);
  file.read_dataset_double_rank_1("total_times", group, results.total_times);

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
  file.read_dataset_string_rank_1("result_block_set", group, results.result_block_set);
}

void CoreResultsDatPersistence::write_block_metadata(HDF5Tool& file, const CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";

  file.createGroup(group);
  file.write_dataset_int_rank_2("result_blocks", group, results.result_blocks);
  file.write_dataset_double_rank_1("total_times", group, results.total_times);

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
  file.write_dataset_string_rank_1("result_block_set", group, results.result_block_set);
}

void CoreResultsDatPersistence::read_result_values(HDF5Tool& file, CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";
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

  const std::string mappings = group + "result_block_c1_data/";

  if (file.nameExists(mappings))
  {
    int index = 0;

    while (file.nameExists(mappings + std::to_string(index) + "/"))
    {
      results.result_block_c1_data.emplace_back();
      file.read_dataset_int_rank_2(std::to_string(index), mappings, results.result_block_c1_data[index]);
      ++index;
    }
  }

  const std::string buckling = group + "buckle_data/";

  if (file.nameExists(buckling))
  {
    int index = 0;

    while (file.nameExists(buckling + std::to_string(index) + "/"))
    {
      results.buckle_data.emplace_back();
      file.read_dataset_double_rank_2(std::to_string(index), buckling, results.buckle_data[index]);
      ++index;
    }
  }
}

void CoreResultsDatPersistence::write_result_values(HDF5Tool& file, const CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";

  if (!results.result_block_data.empty())
  {
    const std::string subgroup = group + "result_block_data/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_block_data.size(); index++)
    {
      file.write_dataset_double_rank_2(std::to_string(index), subgroup, results.result_block_data[index]);
    }
  }

  if (!results.result_block_c1_data.empty())
  {
    const std::string subgroup = group + "result_block_c1_data/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_block_c1_data.size(); index++)
    {
      file.write_dataset_int_rank_2(std::to_string(index), subgroup, results.result_block_c1_data[index]);
    }
  }

  if (!results.buckle_data.empty())
  {
    const std::string subgroup = group + "buckle_data/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.buckle_data.size(); index++)
    {
      file.write_dataset_double_rank_2(std::to_string(index), subgroup, results.buckle_data[index]);
    }
  }
}

bool CoreResultsDatPersistence::read_section_results(HDF5Tool& file, CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";
  const std::string sections = group + "result_section/";

  if (file.nameExists(sections))
  {
    int index = 0;

    while (file.nameExists(sections + std::to_string(index) + "/"))
    {
      results.result_section.emplace_back();
      file.read_dataset_int_rank_1(std::to_string(index), sections, results.result_section[index]);
      ++index;
    }
  }

  file.read_dataset_string_rank_1("result_section_set", group, results.result_section_set);

  // Preserve default labels unless saved labels are read successfully.
  if (file.nameExists(group + "result_section_label"))
  {
    std::vector<std::string> labels;

    if (!file.read_dataset_string_rank_1("result_section_label", group, labels))
    {
      PRINT_ERROR("Unable to read DAT section labels for job %d.\n", results.job_id);
      return false;
    }

    results.result_section_label.swap(labels);
  }

  const std::string values = group + "result_section_data/";

  if (file.nameExists(values))
  {
    int index = 0;

    while (file.nameExists(values + std::to_string(index) + "/"))
    {
      results.result_section_data.emplace_back();
      file.read_dataset_double_rank_1(std::to_string(index), values, results.result_section_data[index]);
      ++index;
    }
  }

  return true;
}

void CoreResultsDatPersistence::write_section_results(HDF5Tool& file, const CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";

  if (!results.result_section.empty())
  {
    const std::string subgroup = group + "result_section/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_section.size(); index++)
    {
      file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.result_section[index]);
    }
  }

  file.write_dataset_string_rank_1("result_section_set", group, results.result_section_set);
  file.write_dataset_string_rank_1("result_section_label", group, results.result_section_label);

  if (!results.result_section_data.empty())
  {
    const std::string subgroup = group + "result_section_data/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.result_section_data.size(); index++)
    {
      file.write_dataset_double_rank_1(std::to_string(index), subgroup, results.result_section_data[index]);
    }
  }
}

void CoreResultsDatPersistence::read_sorted_indices(HDF5Tool& file, CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";
  const std::string column_ids = group + "sorted_c1/";

  if (file.nameExists(column_ids))
  {
    int index = 0;

    while (file.nameExists(column_ids + std::to_string(index) + "/"))
    {
      results.sorted_c1.emplace_back();
      file.read_dataset_int_rank_1(std::to_string(index), column_ids, results.sorted_c1[index]);
      ++index;
    }
  }

  const std::string data_ids = group + "sorted_result_block_c1_data_id/";

  if (file.nameExists(data_ids))
  {
    int index = 0;

    while (file.nameExists(data_ids + std::to_string(index) + "/"))
    {
      results.sorted_result_block_c1_data_id.emplace_back();
      file.read_dataset_int_rank_1(std::to_string(index), data_ids, results.sorted_result_block_c1_data_id[index]);
      ++index;
    }
  }

  const std::string data_types = group + "sorted_result_block_c1_data_type/";

  if (file.nameExists(data_types))
  {
    int index = 0;

    while (file.nameExists(data_types + std::to_string(index) + "/"))
    {
      results.sorted_result_block_c1_data_type.emplace_back();
      file.read_dataset_int_rank_1(std::to_string(index), data_types, results.sorted_result_block_c1_data_type[index]);
      ++index;
    }
  }
}

void CoreResultsDatPersistence::write_sorted_indices(HDF5Tool& file, const CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";

  if (!results.sorted_c1.empty())
  {
    const std::string subgroup = group + "sorted_c1/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.sorted_c1.size(); index++)
    {
      file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_c1[index]);
    }
  }

  if (!results.sorted_result_block_c1_data_id.empty())
  {
    const std::string subgroup = group + "sorted_result_block_c1_data_id/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.sorted_result_block_c1_data_id.size(); index++)
    {
      file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_block_c1_data_id[index]);
    }
  }

  if (!results.sorted_result_block_c1_data_type.empty())
  {
    const std::string subgroup = group + "sorted_result_block_c1_data_type/";
    file.createGroup(subgroup);

    for (std::size_t index = 0; index < results.sorted_result_block_c1_data_type.size(); index++)
    {
      file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_block_c1_data_type[index]);
    }
  }
}

bool CoreResultsDatPersistence::read(HDF5Tool& file, CoreResultsDat& results)
{
  const std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(results.job_id) + "/";

  if (!file.nameExists(group))
  {
    return true;
  }

  read_block_metadata(file, results);
  read_result_values(file, results);

  if (!read_section_results(file, results))
  {
    return false;
  }

  read_sorted_indices(file, results);
  return true;
}

void CoreResultsDatPersistence::write(HDF5Tool& file, const CoreResultsDat& results)
{
  write_block_metadata(file, results);
  write_result_values(file, results);
  write_section_results(file, results);
  write_sorted_indices(file, results);
}
