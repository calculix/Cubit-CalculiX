#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/jobs/CoreJobsPersistence.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/jobs/CoreJobRunPersistence.hpp"
#include <set>
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>
#include <algorithm>
#include <utility>

#include <cstddef>
#include <string>

bool CoreJobsPersistence::read(HDF5Tool& file, CoreJobs& jobs)
{
  try
  {
    decltype(jobs.jobs_data) saved_jobs_data;
    decltype(jobs.output_console) saved_output_console;
    decltype(jobs.cvg) saved_cvg;
    decltype(jobs.sta) saved_sta;
    const std::string group = CalculiXModelSchema::path("Jobs");
    if (file.nameExists(group) && file.nameExists(group + "/jobs_data") && !file.read_dataset_string_rank_2("jobs_data", group, saved_jobs_data))
    {
      PRINT_ERROR("Cannot read CalculiX jobs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!PersistenceTable::keyedRows(saved_jobs_data, 10) || !PersistenceTable::uniqueIds(saved_jobs_data))
    {
      PRINT_ERROR("Cannot read CalculiX jobs: invalid job records.\n");
      return false;
    }
    for (const auto& row : saved_jobs_data)
    {
      for (const int column : {3, 4, 5, 6, 7, 8, 9})
      {
        int value = 0;
        if (!PersistenceTable::id(row[column], value))
        {
          PRINT_ERROR("Cannot read CalculiX jobs: invalid numeric job field.\n");
          return false;
        }
      }
    }

    std::size_t output_console_count = saved_jobs_data.size();
    for (const auto& row : saved_jobs_data)
    {
      int reference = -1;
      if (!PersistenceTable::id(row[5], reference) || reference < -1 || reference > 1000000)
      {
        PRINT_ERROR("Cannot read CalculiX jobs: invalid output_console reference.\n");
        return false;
      }
      if (reference >= 0)
      {
        output_console_count = (std::max)(output_console_count, static_cast<std::size_t>(reference) + 1);
      }
    }
    saved_output_console.resize(output_console_count);
    for (std::size_t index = 0; index < saved_output_console.size(); ++index)
    {
      const std::string subgroup = group + "/output_console";
      const std::string dataset = std::to_string(index);
      if (file.nameExists(subgroup) && file.nameExists(subgroup + "/" + dataset) && !file.read_dataset_string_rank_1(dataset, subgroup, saved_output_console[index]))
      {
        PRINT_ERROR("Cannot read CalculiX job output_console: %s\n", file.last_error().c_str());
        return false;
      }
    }
    std::size_t cvg_count = saved_jobs_data.size();
    for (const auto& row : saved_jobs_data)
    {
      int reference = -1;
      if (!PersistenceTable::id(row[7], reference) || reference < -1 || reference > 1000000)
      {
        PRINT_ERROR("Cannot read CalculiX jobs: invalid cvg reference.\n");
        return false;
      }
      if (reference >= 0)
      {
        cvg_count = (std::max)(cvg_count, static_cast<std::size_t>(reference) + 1);
      }
    }
    saved_cvg.resize(cvg_count);
    for (std::size_t index = 0; index < saved_cvg.size(); ++index)
    {
      const std::string subgroup = group + "/cvg";
      const std::string dataset = std::to_string(index);
      if (file.nameExists(subgroup) && file.nameExists(subgroup + "/" + dataset) && !file.read_dataset_string_rank_1(dataset, subgroup, saved_cvg[index]))
      {
        PRINT_ERROR("Cannot read CalculiX job cvg: %s\n", file.last_error().c_str());
        return false;
      }
    }
    std::size_t sta_count = saved_jobs_data.size();
    for (const auto& row : saved_jobs_data)
    {
      int reference = -1;
      if (!PersistenceTable::id(row[8], reference) || reference < -1 || reference > 1000000)
      {
        PRINT_ERROR("Cannot read CalculiX jobs: invalid sta reference.\n");
        return false;
      }
      if (reference >= 0)
      {
        sta_count = (std::max)(sta_count, static_cast<std::size_t>(reference) + 1);
      }
    }
    saved_sta.resize(sta_count);
    for (std::size_t index = 0; index < saved_sta.size(); ++index)
    {
      const std::string subgroup = group + "/sta";
      const std::string dataset = std::to_string(index);
      if (file.nameExists(subgroup) && file.nameExists(subgroup + "/" + dataset) && !file.read_dataset_string_rank_1(dataset, subgroup, saved_sta[index]))
      {
        PRINT_ERROR("Cannot read CalculiX job sta: %s\n", file.last_error().c_str());
        return false;
      }
    }
    std::map<int, std::shared_ptr<const CalculiXJobRunRecord>> saved_runs;
    std::string runError;
    if (!CoreJobRunPersistence::read(file, saved_runs, runError))
    {
      PRINT_ERROR("Cannot read job run archive: %s\n", runError.c_str());
      return false;
    }
    std::set<int> ids;
    for (const auto& row : saved_jobs_data)
    {
      ids.insert(std::stoi(row[0]));
    }
    for (const auto& run : saved_runs)
    {
      if (ids.count(run.first) == 0)
      {
        PRINT_ERROR("Archived launch refers to a missing job.\n");
        return false;
      }
    }
    jobs.jobs_data.swap(saved_jobs_data);
    jobs.runRecords.swap(saved_runs);
    jobs.preparedRuns.clear();
    jobs.preparedRunErrors.clear();
    jobs.unsupportedPreparedRuns.clear();
    jobs.clear_monitor_executions();
    jobs.clear_job_progress();
    jobs.output_console.swap(saved_output_console);
    jobs.cvg.swap(saved_cvg);
    jobs.sta.swap(saved_sta);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX jobs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX jobs: %s\n", error.what());
    return false;
  }
}

bool CoreJobsPersistence::write(HDF5Tool& file, const CoreJobs& jobs)
{
  try
  {
    if (!PersistenceTable::keyedRows(jobs.jobs_data, 10) || !PersistenceTable::uniqueIds(jobs.jobs_data))
    {
      PRINT_ERROR("Cannot write CalculiX jobs: invalid job records.\n");
      return false;
    }
    for (const auto& row : jobs.jobs_data)
    {
      for (const int column : {3, 4, 5, 6, 7, 8, 9})
      {
        int value = 0;
        if (!PersistenceTable::id(row[column], value))
        {
          PRINT_ERROR("Cannot write CalculiX jobs: invalid numeric job field.\n");
          return false;
        }
      }
    }

    for (const auto& row : jobs.jobs_data)
    {
      for (const auto& reference : {std::make_pair(5, jobs.output_console.size()), std::make_pair(7, jobs.cvg.size()), std::make_pair(8, jobs.sta.size())})
      {
        int index = -1;
        if (!PersistenceTable::id(row[reference.first], index) || index < -1 || (index >= 0 && static_cast<std::size_t>(index) >= reference.second))
        {
          PRINT_ERROR("Cannot save CalculiX jobs: invalid console/status reference.\n");
          return false;
        }
      }
    }

    if (!file.ensureGroup(CalculiXModelSchema::path("Jobs")))
    {
      PRINT_ERROR("Cannot save CalculiX jobs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Jobs/output_console")))
    {
      PRINT_ERROR("Cannot save CalculiX jobs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Jobs/cvg")))
    {
      PRINT_ERROR("Cannot save CalculiX jobs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Jobs/sta")))
    {
      PRINT_ERROR("Cannot save CalculiX jobs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("jobs_data", CalculiXModelSchema::path("Jobs"), jobs.jobs_data))
    {
      PRINT_ERROR("Cannot save CalculiX jobs: %s\n", file.last_error().c_str());
      return false;
    }
    for (std::size_t index = 0; index < jobs.output_console.size(); ++index)
    {
      if (!file.write_dataset_string_rank_1(std::to_string(index), CalculiXModelSchema::path("Jobs/output_console"), jobs.output_console[index]))
      {
        PRINT_ERROR("Cannot save CalculiX job output_console: %s\n", file.last_error().c_str());
        return false;
      }
    }
    for (std::size_t index = 0; index < jobs.cvg.size(); ++index)
    {
      if (!file.write_dataset_string_rank_1(std::to_string(index), CalculiXModelSchema::path("Jobs/cvg"), jobs.cvg[index]))
      {
        PRINT_ERROR("Cannot save CalculiX job cvg: %s\n", file.last_error().c_str());
        return false;
      }
    }
    for (std::size_t index = 0; index < jobs.sta.size(); ++index)
    {
      if (!file.write_dataset_string_rank_1(std::to_string(index), CalculiXModelSchema::path("Jobs/sta"), jobs.sta[index]))
      {
        PRINT_ERROR("Cannot save CalculiX job sta: %s\n", file.last_error().c_str());
        return false;
      }
    }
    std::string runError;
    if (!CoreJobRunPersistence::write(file, jobs.runRecords, runError))
    {
      PRINT_ERROR("Cannot save job run archive: %s\n", runError.c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX jobs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX jobs: %s\n", error.what());
    return false;
  }
}
