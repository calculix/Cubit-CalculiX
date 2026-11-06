#ifndef CALCULIXMODELSCHEMA_HPP
#define CALCULIXMODELSCHEMA_HPP

#include "Framework/Storage/ModelFileMetadata.hpp"
#include <string>

namespace CalculiXModelSchema
{
inline constexpr int SchemaVersion = 1;

// New model namespace; older releases remain responsible for legacy files.
inline std::string path(const std::string& suffix)
{
  const std::string root = std::string(CubitSimModelFile::Root) + "/workspaces/calculix";
  if (suffix.empty())
  {
    return root;
  }
  return root + "/" + suffix;
}
}

#endif
