#ifndef CUBITSIM_FRAMEWORKPATHS_HPP
#define CUBITSIM_FRAMEWORKPATHS_HPP

#include "Framework/Storage/StorageExport.hpp"
#include <string>

struct CUBITSIM_STORAGE_EXPORT FrameworkPaths
{
  // componentAnchor must belong to the loaded component/host library.
  // All component resources are assumed to be deployed beside that library.
  static bool resolve(const void* componentAnchor, FrameworkPaths& paths, std::string& error);

  std::string componentDirectory;
  std::string optionsFilename;
};

#endif
