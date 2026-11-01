#include "Core/application/CoreModelMetadataPersistence.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"

#include <vector>

void CoreModelMetadataPersistence::read(HDF5Tool& file)
{
  std::vector<std::string> general;
  file.read_dataset_string_rank_1("general", "Cubit-CalculiX", general);

  if (!general.empty())
  {
    const std::string log = "Save file was created with Cubit-CalculiX " + general[0] + "\n";
    PRINT_INFO("%s", log.c_str());
  }
}

void CoreModelMetadataPersistence::write(HDF5Tool& file, const std::string& version)
{
  file.createGroup("Cubit-CalculiX");

  const std::vector<std::string> general = {version};
  file.write_dataset_string_rank_1("general", "Cubit-CalculiX", general);

  file.createGroup("Cubit-CalculiX/Core");
}
