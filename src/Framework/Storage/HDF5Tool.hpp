#ifndef HDF5TOOL_HPP
#define HDF5TOOL_HPP

#include "StorageExport.hpp"

#include <vector>
#include <string>
#include <chrono>
#include <H5Cpp.h>

class CUBITSIM_STORAGE_EXPORT HDF5Tool
{

public:
  HDF5Tool(std::string filename, bool createIfMissing = true);
  ~HDF5Tool() noexcept;
  HDF5Tool(const HDF5Tool&) = delete;
  HDF5Tool& operator=(const HDF5Tool&) = delete;
  bool close() noexcept;
  bool ensureGroup(const std::string& groupname);
  const std::string& last_error() const;

  bool nameExists(std::string name);
  bool createGroup(std::string groupname);
  bool renameGroup(std::string groupname, std::string new_groupname);
  bool deleteGroup(std::string groupname);
  std::vector<std::string> getGroupsFromFile(); // gets all group paths from the file

  bool deleteDataset(std::string name,std::string groupname);

  //int
  bool read_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> &data); // read dataset
  bool read_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> &data); // read dataset
  bool write_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> data); // create dataset
  bool write_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> data); // create dataset
  //double
  bool read_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> &data); // read dataset
  bool read_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> &data); // read dataset
  bool write_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> data); // create dataset
  bool write_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> data); // create dataset
  
  //string
  bool read_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> &data); // read dataset
  bool read_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> &data); // read dataset
  bool write_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> data); // create dataset
  bool write_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> data); // create dataset

  H5::H5File* file = nullptr;
private:
  std::string lastError;
};

#endif // HDF5TOOL_HPP