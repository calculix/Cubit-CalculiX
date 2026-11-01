#include "Core/jobs/CoreJobsPersistence.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "HDF5Tool.hpp"

#include <cstddef>
#include <string>

void CoreJobsPersistence::read(HDF5Tool& file, CoreJobs& jobs)
{
  if (file.read_dataset_string_rank_2("jobs_data", "Cubit-CalculiX/Jobs", jobs.jobs_data))
  {
    for (std::size_t i = 0; i < jobs.jobs_data.size(); i++)
    {
      jobs.output_console.emplace_back();
      jobs.cvg.emplace_back();
      jobs.sta.emplace_back();

      const std::string dataset = std::to_string(i);
      file.read_dataset_string_rank_1(dataset, "Cubit-CalculiX/Jobs/output_console", jobs.output_console[i]);
      file.read_dataset_string_rank_1(dataset, "Cubit-CalculiX/Jobs/cvg", jobs.cvg[i]);
      file.read_dataset_string_rank_1(dataset, "Cubit-CalculiX/Jobs/sta", jobs.sta[i]);
    }
  }
}

void CoreJobsPersistence::write(HDF5Tool& file, const CoreJobs& jobs)
{
  file.createGroup("Cubit-CalculiX/Jobs");
  file.createGroup("Cubit-CalculiX/Jobs/output_console");
  file.createGroup("Cubit-CalculiX/Jobs/cvg");
  file.createGroup("Cubit-CalculiX/Jobs/sta");
  file.write_dataset_string_rank_2("jobs_data", "Cubit-CalculiX/Jobs", jobs.jobs_data);

  for (std::size_t i = 0; i < jobs.jobs_data.size(); i++)
  {
    const std::string dataset = std::to_string(i);
    file.write_dataset_string_rank_1(dataset, "Cubit-CalculiX/Jobs/output_console", jobs.output_console[i]);
    file.write_dataset_string_rank_1(dataset, "Cubit-CalculiX/Jobs/cvg", jobs.cvg[i]);
    file.write_dataset_string_rank_1(dataset, "Cubit-CalculiX/Jobs/sta", jobs.sta[i]);
  }
}
