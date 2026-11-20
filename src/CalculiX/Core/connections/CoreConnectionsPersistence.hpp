#pragma once
class CoreConnections;
class HDF5Tool;

class CoreConnectionsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreConnections& connections);
  static bool write(HDF5Tool& file, const CoreConnections& connections);
};
