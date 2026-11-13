#pragma once
#include "CalculiXJobRunRecord.hpp"
class HDF5Tool;

class CoreJobRunPersistence
{
public:
  CoreJobRunPersistence();
  // Reads into a candidate map; caller commits together with the other job data.
  static bool read(HDF5Tool& file, std::map<int, std::shared_ptr<const CalculiXJobRunRecord>>& output, std::string& error);
  static bool write(HDF5Tool& file, const std::map<int, std::shared_ptr<const CalculiXJobRunRecord>>& records, std::string& error);
};
