#pragma once
#include "Framework/ModelExport.hpp"
#include <string>

class HDF5Tool;
namespace SimUnits {
class CoreFrameworkUnits;
class CUBITSIM_MODEL_EXPORT CoreFrameworkUnitsPersistence {
public:
  CoreFrameworkUnitsPersistence();
  bool read(HDF5Tool& file, CoreFrameworkUnits& units, std::string& error) const;
  bool write(HDF5Tool& file, const CoreFrameworkUnits& units, std::string& error) const;
  bool read(const std::string& filename, CoreFrameworkUnits& units, std::string& error) const;
  bool write(const std::string& filename, const CoreFrameworkUnits& units, std::string& error) const;
};
}
