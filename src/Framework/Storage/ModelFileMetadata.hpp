#ifndef CUBITSIM_MODELFILEMETADATA_HPP
#define CUBITSIM_MODELFILEMETADATA_HPP

#include "StorageExport.hpp"
#include <string>

class HDF5Tool;

namespace CubitSimModelFile
{
inline constexpr const char* Root = "cubit-sim";
inline constexpr const char* Metadata = "cubit-sim/metadata";
inline constexpr int SchemaVersion = 1;

// These functions operate on an already-open model file; the caller owns it.
// Workspace schemas are owned and validated by the corresponding workspace.
// Versions other than SchemaVersion are rejected. No legacy reader exists.
CUBITSIM_STORAGE_EXPORT bool read(HDF5Tool& file, std::string& applicationVersion, std::string& error);
CUBITSIM_STORAGE_EXPORT bool write(HDF5Tool& file, const std::string& applicationVersion, std::string& error);
}

#endif
