#ifndef HDF5Tool_HPP
#define HDF5Tool_HPP

#include <hdf5.h>

#include <string>
#include <vector>

// C API version of HDF5Tool. It needs hdf5.dll, but not hdf5_cpp.dll.
class HDF5Tool
{
public:
  explicit HDF5Tool(std::string filename);
  ~HDF5Tool();

  HDF5Tool(const HDF5Tool&) = delete;
  HDF5Tool& operator=(const HDF5Tool&) = delete;

  bool nameExists(std::string name);
  bool createGroup(std::string groupname);
  bool renameGroup(std::string groupname, std::string new_groupname);
  bool deleteGroup(std::string groupname);
  std::vector<std::string> getGroupsFromFile();
  bool deleteDataset(std::string name, std::string groupname);

  bool read_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int>& data);
  bool read_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>>& data);
  bool write_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> data);
  bool write_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> data);

  bool read_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double>& data);
  bool read_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>>& data);
  bool write_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> data);
  bool write_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> data);

  bool read_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string>& data);
  bool read_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>>& data);
  bool write_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> data);
  bool write_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> data);

  hid_t file = H5I_INVALID_HID;
};

#endif // HDF5Tool_HPP
