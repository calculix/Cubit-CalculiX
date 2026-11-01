#include "Core/results/CoreResultsPersistence.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/frd/CoreResultsFrdPersistence.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/results/dat/CoreResultsDatPersistence.hpp"
#include "HDF5Tool.hpp"

void CoreResultsPersistence::read_registry(HDF5Tool& file, CoreResults& results)
{
  file.read_dataset_int_rank_2("results_data", "Cubit-CalculiX/Results", results.results_data);
}

void CoreResultsPersistence::write_registry(HDF5Tool& file, const CoreResults& results)
{
  file.createGroup("Cubit-CalculiX/Results");
  file.write_dataset_int_rank_2("results_data", "Cubit-CalculiX/Results", results.results_data);
  file.createGroup("Cubit-CalculiX/Results/Frd");
  file.createGroup("Cubit-CalculiX/Results/Dat");
}

void CoreResultsPersistence::initialize_readers(CoreResults& results)
{
  for (const auto& entry : results.results_data)
  {
    results.create_frd_dat(entry[1]);
  }
}

bool CoreResultsPersistence::read_loaded_results(HDF5Tool& file, CoreResults& results)
{
  initialize_readers(results);

  for (auto& frd : results.frd_data)
  {
    CoreResultsFrdPersistence::read(file, frd);
  }

  for (auto& dat : results.dat_data)
  {
    if (!CoreResultsDatPersistence::read(file, dat))
    {
      return false;
    }
  }

  return true;
}

void CoreResultsPersistence::write(HDF5Tool& file, const CoreResults& results, bool save_loaded_results)
{
  write_registry(file, results);

  if (!save_loaded_results)
  {
    return;
  }

  for (const auto& frd : results.frd_data)
  {
    CoreResultsFrdPersistence::write(file, frd);
  }

  for (const auto& dat : results.dat_data)
  {
    CoreResultsDatPersistence::write(file, dat);
  }
}
