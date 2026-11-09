#pragma once
#include "StorageExport.hpp"
#include <functional>
#include <string>

class HDF5Tool;

// Shared by writers of cubitsim_user_options.h5. Never replace that file without
// taking the same sidecar lock and closing every HDF5 handle before commit.
class CUBITSIM_STORAGE_EXPORT Hdf5FileTransaction
{
public:
  Hdf5FileTransaction();
  using Operation = std::function<bool(HDF5Tool&, bool, std::string&)>;
  bool update(const std::string& filename, const Operation& operation, std::string& error) const;
};
