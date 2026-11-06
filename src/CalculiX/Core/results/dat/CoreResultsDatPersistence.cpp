#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/results/dat/CoreResultsDatPersistence.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"

#include <cstddef>
#include <string>
#include <vector>

#include "PersistenceTable.hpp"
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <charconv>
#include <cmath>
#include <set>

namespace
{
  struct SavedDatData
  {
    int job_id = -1;
    std::vector<DatResultTable> result_tables;
    std::vector<std::string> diagnostics;
    decltype(CoreResultsDat::buckle_data) buckle_data;
    decltype(CoreResultsDat::result_block_c1_data) result_block_c1_data;
    decltype(CoreResultsDat::result_block_components) result_block_components;
    decltype(CoreResultsDat::result_block_data) result_block_data;
    decltype(CoreResultsDat::result_block_set) result_block_set;
    decltype(CoreResultsDat::result_block_type) result_block_type;
    decltype(CoreResultsDat::result_blocks) result_blocks;
    decltype(CoreResultsDat::result_section) result_section;
    decltype(CoreResultsDat::result_section_data) result_section_data;
    decltype(CoreResultsDat::result_section_label) result_section_label;
    decltype(CoreResultsDat::result_section_set) result_section_set;
    decltype(CoreResultsDat::sorted_c1) sorted_c1;
    decltype(CoreResultsDat::sorted_result_block_c1_data_id) sorted_result_block_c1_data_id;
    decltype(CoreResultsDat::sorted_result_block_c1_data_type) sorted_result_block_c1_data_type;
    decltype(CoreResultsDat::total_times) total_times;
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

  bool read_block_metadata(HDF5Tool& file, SavedDatData& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";

      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "result_blocks") && !file.read_dataset_int_rank_2("result_blocks", group, results.result_blocks))
      {
        PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "total_times") && !file.read_dataset_double_rank_1("total_times", group, results.total_times))
      {
        PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      const std::string components = group + "result_block_components/";

      if (file.nameExists(components))
      {
        for (const int index : savedIndices(file, components))
        {
          results.result_block_components.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(components) && file.nameExists(components + std::string("/") + std::to_string(index)) && !file.read_dataset_string_rank_1(std::to_string(index), components, results.result_block_components[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "result_block_type") && !file.read_dataset_string_rank_1("result_block_type", group, results.result_block_type))
      {
        PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "result_block_set") && !file.read_dataset_string_rank_1("result_block_set", group, results.result_block_set))
      {
        PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_block_metadata: %s\n", error.what());
      return false;
    }
  }

  bool write_block_metadata(HDF5Tool& file, const CoreResultsDat& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";

      if (!file.ensureGroup(group))
      {
        PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_int_rank_2("result_blocks", group, results.result_blocks))
      {
        PRINT_ERROR("Cannot save CalculiX Dat write_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_double_rank_1("total_times", group, results.total_times))
      {
        PRINT_ERROR("Cannot save CalculiX Dat write_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      if (!results.result_block_components.empty())
      {
        const std::string subgroup = group + "result_block_components/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_block_components.size(); index++)
        {
          if (!file.write_dataset_string_rank_1(std::to_string(index), subgroup, results.result_block_components[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!file.write_dataset_string_rank_1("result_block_type", group, results.result_block_type))
      {
        PRINT_ERROR("Cannot save CalculiX Dat write_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_string_rank_1("result_block_set", group, results.result_block_set))
      {
        PRINT_ERROR("Cannot save CalculiX Dat write_block_metadata for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_block_metadata: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_block_metadata: %s\n", error.what());
      return false;
    }
  }

  bool read_result_values(HDF5Tool& file, SavedDatData& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";
      const std::string values = group + "result_block_data/";

      if (file.nameExists(values))
      {
        for (const int index : savedIndices(file, values))
        {
          results.result_block_data.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(values) && file.nameExists(values + std::string("/") + std::to_string(index)) && !file.read_dataset_double_rank_2(std::to_string(index), values, results.result_block_data[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_result_values for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      const std::string mappings = group + "result_block_c1_data/";

      if (file.nameExists(mappings))
      {
        for (const int index : savedIndices(file, mappings))
        {
          results.result_block_c1_data.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(mappings) && file.nameExists(mappings + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_2(std::to_string(index), mappings, results.result_block_c1_data[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_result_values for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      const std::string buckling = group + "buckle_data/";

      if (file.nameExists(buckling))
      {
        for (const int index : savedIndices(file, buckling))
        {
          results.buckle_data.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(buckling) && file.nameExists(buckling + std::string("/") + std::to_string(index)) && !file.read_dataset_double_rank_2(std::to_string(index), buckling, results.buckle_data[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_result_values for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_result_values: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_result_values: %s\n", error.what());
      return false;
    }
  }

  bool write_result_values(HDF5Tool& file, const CoreResultsDat& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";

      if (!results.result_block_data.empty())
      {
        const std::string subgroup = group + "result_block_data/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_block_data.size(); index++)
        {
          if (!file.write_dataset_double_rank_2(std::to_string(index), subgroup, results.result_block_data[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_result_values for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!results.result_block_c1_data.empty())
      {
        const std::string subgroup = group + "result_block_c1_data/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_block_c1_data.size(); index++)
        {
          if (!file.write_dataset_int_rank_2(std::to_string(index), subgroup, results.result_block_c1_data[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_result_values for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!results.buckle_data.empty())
      {
        const std::string subgroup = group + "buckle_data/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.buckle_data.size(); index++)
        {
          if (!file.write_dataset_double_rank_2(std::to_string(index), subgroup, results.buckle_data[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_result_values for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_result_values: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_result_values: %s\n", error.what());
      return false;
    }
  }

  bool read_section_results(HDF5Tool& file, SavedDatData& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";
      const std::string sections = group + "result_section/";

      if (file.nameExists(sections))
      {
        for (const int index : savedIndices(file, sections))
        {
          results.result_section.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(sections) && file.nameExists(sections + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_1(std::to_string(index), sections, results.result_section[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (file.nameExists(group) && file.nameExists(group + std::string("/") + "result_section_set") && !file.read_dataset_string_rank_1("result_section_set", group, results.result_section_set))
      {
        PRINT_ERROR("Cannot read CalculiX Dat read_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

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
        for (const int index : savedIndices(file, values))
        {
          results.result_section_data.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(values) && file.nameExists(values + std::string("/") + std::to_string(index)) && !file.read_dataset_double_rank_1(std::to_string(index), values, results.result_section_data[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_section_results: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_section_results: %s\n", error.what());
      return false;
    }
  }

  bool write_section_results(HDF5Tool& file, const CoreResultsDat& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";

      if (!results.result_section.empty())
      {
        const std::string subgroup = group + "result_section/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_section.size(); index++)
        {
          if (!file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.result_section[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!file.write_dataset_string_rank_1("result_section_set", group, results.result_section_set))
      {
        PRINT_ERROR("Cannot save CalculiX Dat write_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }
      if (!file.write_dataset_string_rank_1("result_section_label", group, results.result_section_label))
      {
        PRINT_ERROR("Cannot save CalculiX Dat write_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
        return false;
      }

      if (!results.result_section_data.empty())
      {
        const std::string subgroup = group + "result_section_data/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.result_section_data.size(); index++)
        {
          if (!file.write_dataset_double_rank_1(std::to_string(index), subgroup, results.result_section_data[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_section_results for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_section_results: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_section_results: %s\n", error.what());
      return false;
    }
  }

  bool read_sorted_indices(HDF5Tool& file, SavedDatData& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";
      const std::string column_ids = group + "sorted_c1/";

      if (file.nameExists(column_ids))
      {
        for (const int index : savedIndices(file, column_ids))
        {
          results.sorted_c1.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(column_ids) && file.nameExists(column_ids + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_1(std::to_string(index), column_ids, results.sorted_c1[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      const std::string data_ids = group + "sorted_result_block_c1_data_id/";

      if (file.nameExists(data_ids))
      {
        for (const int index : savedIndices(file, data_ids))
        {
          results.sorted_result_block_c1_data_id.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(data_ids) && file.nameExists(data_ids + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_1(std::to_string(index), data_ids, results.sorted_result_block_c1_data_id[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      const std::string data_types = group + "sorted_result_block_c1_data_type/";

      if (file.nameExists(data_types))
      {
        for (const int index : savedIndices(file, data_types))
        {
          results.sorted_result_block_c1_data_type.resize(static_cast<std::size_t>(index) + 1);
          if (file.nameExists(data_types) && file.nameExists(data_types + std::string("/") + std::to_string(index)) && !file.read_dataset_int_rank_1(std::to_string(index), data_types, results.sorted_result_block_c1_data_type[index]))
          {
            PRINT_ERROR("Cannot read CalculiX Dat read_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_sorted_indices: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot read CalculiX Dat read_sorted_indices: %s\n", error.what());
      return false;
    }
  }

  bool write_sorted_indices(HDF5Tool& file, const CoreResultsDat& results)
  {
    try
    {
      const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id) + "/";

      if (!results.sorted_c1.empty())
      {
        const std::string subgroup = group + "sorted_c1/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.sorted_c1.size(); index++)
        {
          if (!file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_c1[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!results.sorted_result_block_c1_data_id.empty())
      {
        const std::string subgroup = group + "sorted_result_block_c1_data_id/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.sorted_result_block_c1_data_id.size(); index++)
        {
          if (!file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_block_c1_data_id[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }

      if (!results.sorted_result_block_c1_data_type.empty())
      {
        const std::string subgroup = group + "sorted_result_block_c1_data_type/";
        if (!file.ensureGroup(subgroup))
        {
          PRINT_ERROR("Cannot save CalculiX Dat result group for job %d: %s\n", results.job_id, file.last_error().c_str());
          return false;
        }

        for (std::size_t index = 0; index < results.sorted_result_block_c1_data_type.size(); index++)
        {
          if (!file.write_dataset_int_rank_1(std::to_string(index), subgroup, results.sorted_result_block_c1_data_type[index]))
          {
            PRINT_ERROR("Cannot save CalculiX Dat write_sorted_indices for job %d: %s\n", results.job_id, file.last_error().c_str());
            return false;
          }
        }
      }
      return true;
    }
    catch (const H5::Exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_sorted_indices: %s\n", error.getDetailMsg().c_str());
      return false;
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Cannot write CalculiX Dat write_sorted_indices: %s\n", error.what());
      return false;
    }
  }

  template<class Data>
  bool valid_named_tables(const Data& results)
  {
    for (const auto& table : results.result_tables)
    {
      const std::set<std::string> names(table.columns.begin(), table.columns.end());
      if (table.key.empty() || !std::isfinite(table.time) || table.columns.empty() ||
        names.size() != table.columns.size() || names.count("") || names.count("RecordIndex") || names.count("Valid") ||
        !std::all_of(table.rows.begin(), table.rows.end(), [&](const auto& row)
        {
          return row.size() == table.columns.size();
        }))
      {
        return false;
      }
    }
    return true;
  }

  bool read_named_tables(HDF5Tool& file, const std::string& group, SavedDatData& results)
  {
    if (!file.nameExists(group + "/NamedTableCount"))
    {
      return true; // Files saved before named tables were introduced.
    }
    std::vector<int> count;
    if (!file.read_dataset_int_rank_1("NamedTableCount", group, count) || count.size() != 1 || count[0] < 0 || count[0] > 1000000)
    {
      return false;
    }
    results.result_tables.resize(static_cast<std::size_t>(count[0]));
    for (std::size_t i = 0; i < results.result_tables.size(); ++i)
    {
      auto& table = results.result_tables[i];
      const auto path = group + "/NamedTables/" + std::to_string(i) + "/";
      std::vector<std::string> metadata;
      std::vector<int> identity;
      std::vector<double> time;
      if (!file.read_dataset_string_rank_1("Metadata", path, metadata) || metadata.size() != 6 ||
        !file.read_dataset_int_rank_1("Identity", path, identity) || identity.size() != 2 ||
        !file.read_dataset_double_rank_1("Time", path, time) || time.size() != 1 ||
        !file.read_dataset_string_rank_1("Columns", path, table.columns) ||
        !file.read_dataset_double_rank_2("Rows", path, table.rows))
      {
        return false;
      }
      table.key = metadata[0];
      table.name = metadata[1];
      table.set_name = metadata[2];
      table.slave_set = metadata[3];
      table.master_set = metadata[4];
      const auto parsed = std::from_chars(metadata[5].data(), metadata[5].data() + metadata[5].size(), table.source_line);
      if (metadata[5].empty() || parsed.ec != std::errc{} || parsed.ptr != metadata[5].data() + metadata[5].size())
      {
        return false;
      }
      table.step = identity[0];
      table.increment = identity[1];
      table.time = time[0];
    }
    if (file.nameExists(group + "/ReaderDiagnostics") && !file.read_dataset_string_rank_1("ReaderDiagnostics", group, results.diagnostics))
    {
      return false;
    }
    return valid_named_tables(results);
  }

  bool write_named_tables(HDF5Tool& file, const std::string& group, const CoreResultsDat& results)
  {
    if (results.result_tables.size() > 1000000 || !file.ensureGroup(group) ||
      !file.write_dataset_int_rank_1("NamedTableCount", group, {static_cast<int>(results.result_tables.size())}) ||
      !file.write_dataset_string_rank_1("ReaderDiagnostics", group, results.diagnostics))
    {
      return false;
    }
    for (std::size_t i = 0; i < results.result_tables.size(); ++i)
    {
      const auto& table = results.result_tables[i];
      const auto path = group + "/NamedTables/" + std::to_string(i) + "/";
      if (!file.ensureGroup(path) ||
        !file.write_dataset_string_rank_1("Metadata", path, {table.key, table.name, table.set_name, table.slave_set, table.master_set, std::to_string(table.source_line)}) ||
        !file.write_dataset_int_rank_1("Identity", path, {table.step, table.increment}) ||
        !file.write_dataset_double_rank_1("Time", path, {table.time}) ||
        !file.write_dataset_string_rank_1("Columns", path, table.columns) ||
        !file.write_dataset_double_rank_2("Rows", path, table.rows))
      {
        return false;
      }
    }
    return true;
  }

  template<class Data>
  bool validDatData(const Data& results)
  {
    if (!valid_named_tables(results))
    {
      return false;
    }
    if (!PersistenceTable::rows(results.result_blocks, 7) || !PersistenceTable::rows(results.result_section, 2))
    {
      return false;
    }
    for (const auto& block : results.result_blocks)
    {
      if (!inRange(block[1], results.total_times.size()) || !inRange(block[2], results.result_block_type.size()) || !inRange(block[3], results.result_block_set.size()) || !inRange(block[4], results.result_block_data.size()) || !inRange(block[4], results.result_block_components.size()) || !inRange(block[4], results.result_block_c1_data.size()))
      {
        return false;
      }
      const auto& values = results.result_block_data[block[4]];
      if (!PersistenceTable::rows(values, results.result_block_components[block[4]].size()))
      {
        return false;
      }
      for (const auto& mapping : results.result_block_c1_data[block[4]])
      {
        if (mapping.size() < 3 || !inRange(mapping[1], values.size()))
        {
          return false;
        }
      }
    }
    for (const auto& section : results.result_section)
    {
      if (!inRange(section[0], results.result_section_set.size()) || !inRange(section[1], results.result_section_data.size()))
      {
        return false;
      }
    }
    if (results.sorted_c1.size() != results.sorted_result_block_c1_data_id.size() || results.sorted_c1.size() != results.sorted_result_block_c1_data_type.size())
    {
      return false;
    }
    for (std::size_t block = 0; block < results.sorted_c1.size(); ++block)
    {
      const auto& ids = results.sorted_c1[block];
      const auto& indices = results.sorted_result_block_c1_data_id[block];
      const auto& types = results.sorted_result_block_c1_data_type[block];
      if (ids.size() != indices.size() || ids.size() != types.size() || !std::is_sorted(ids.begin(), ids.end()) || block >= results.result_block_data.size())
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
    for (const auto& values : results.result_section_data)
    {
      if (values.size() > results.result_section_label.size())
      {
        return false;
      }
    }
    return true;
  }
}

bool CoreResultsDatPersistence::read(HDF5Tool& file, CoreResultsDat& results)
{
  try
  {
    const std::string group = CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id);
    if (!file.nameExists(group))
    {
      return true;
    }
    SavedDatData saved;
    saved.job_id = results.job_id;
    saved.result_section_label = results.result_section_label;
    if (!read_block_metadata(file, saved))
    {
      return false;
    }
    if (!read_result_values(file, saved))
    {
      return false;
    }
    if (!read_section_results(file, saved))
    {
      return false;
    }
    if (!read_sorted_indices(file, saved))
    {
      return false;
    }
    if (!read_named_tables(file, group, saved))
    {
      PRINT_ERROR("Cannot read DAT named tables for job %d.\n", results.job_id);
      return false;
    }
    if (!validDatData(saved))
    {
      PRINT_ERROR("Cannot read CalculiX Dat results for job %d: invalid data shape or result reference.\n", results.job_id);
      return false;
    }
    results.result_tables.swap(saved.result_tables);
    results.diagnostics.swap(saved.diagnostics);
    results.buckle_data.swap(saved.buckle_data);
    results.result_block_c1_data.swap(saved.result_block_c1_data);
    results.result_block_components.swap(saved.result_block_components);
    results.result_block_data.swap(saved.result_block_data);
    results.result_block_set.swap(saved.result_block_set);
    results.result_block_type.swap(saved.result_block_type);
    results.result_blocks.swap(saved.result_blocks);
    results.result_section.swap(saved.result_section);
    results.result_section_data.swap(saved.result_section_data);
    results.result_section_label.swap(saved.result_section_label);
    results.result_section_set.swap(saved.result_section_set);
    results.sorted_c1.swap(saved.sorted_c1);
    results.sorted_result_block_c1_data_id.swap(saved.sorted_result_block_c1_data_id);
    results.sorted_result_block_c1_data_type.swap(saved.sorted_result_block_c1_data_type);
    results.total_times.swap(saved.total_times);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Dat results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Dat results: %s\n", error.what());
    return false;
  }
}

bool CoreResultsDatPersistence::write(HDF5Tool& file, const CoreResultsDat& results)
{
  try
  {
    if (!validDatData(results))
    {
      PRINT_ERROR("Cannot save CalculiX Dat results for job %d: invalid data shape or result reference.\n", results.job_id);
      return false;
    }
    if (!write_named_tables(file, CalculiXModelSchema::path("Results/Dat/") + std::to_string(results.job_id), results))
    {
      PRINT_ERROR("Cannot save DAT named tables for job %d.\n", results.job_id);
      return false;
    }
    if (!write_block_metadata(file, results))
    {
      return false;
    }
    if (!write_result_values(file, results))
    {
      return false;
    }
    if (!write_section_results(file, results))
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
    PRINT_ERROR("Cannot write CalculiX Dat results: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Dat results: %s\n", error.what());
    return false;
  }
}
