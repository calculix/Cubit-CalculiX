#ifndef HDF5TOOL_HPP
#define HDF5TOOL_HPP

#include <vector>
#include <string>
#include <chrono>
#include <H5Cpp.h>

class HDF5Tool
{

public:
  HDF5Tool(std::string filename);
  ~HDF5Tool();

  bool nameExists(std::string name);
  bool createGroup(std::string groupname);
  bool renameGroup(std::string groupname, std::string new_groupname);
  bool deleteGroup(std::string groupname);

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

  H5::H5File *file;
};

#endif // HDF5TOOL_HPP