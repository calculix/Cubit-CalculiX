#include "HDF5Tool.hpp"

#include <cerrno>
#include <limits>
#include <stdexcept>
#include <utility>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace
{
class Handle
{
public:
  Handle(hid_t value, herr_t (*close_function)(hid_t))
    : value_(value), close_function_(close_function) {}

  ~Handle()
  {
    if (value_ >= 0)
      close_function_(value_);
  }

  Handle(const Handle&) = delete;
  Handle& operator=(const Handle&) = delete;

  Handle(Handle&& other) noexcept
    : value_(other.value_), close_function_(other.close_function_)
  {
    other.value_ = H5I_INVALID_HID;
  }

  hid_t get() const { return value_; }
  explicit operator bool() const { return value_ >= 0; }

private:
  hid_t value_;
  herr_t (*close_function_)(hid_t);
};

bool dataset_exists(hid_t group, const std::string& name)
{
  return H5Lexists(group, name.c_str(), H5P_DEFAULT) > 0;
}

bool valid_size(hsize_t count)
{
  return count <= static_cast<hsize_t>(std::numeric_limits<size_t>::max());
}

bool valid_matrix_size(hsize_t rows, hsize_t columns)
{
  return valid_size(rows) && valid_size(columns) &&
         (columns == 0 || rows <= std::numeric_limits<size_t>::max() / columns);
}

template<typename T>
bool read_numeric_1(hid_t file, const std::string& name,
                    const std::string& groupname, hid_t datatype,
                    std::vector<T>& data)
{
  if (!data.empty() || file < 0)
    return false;

  Handle group(H5Gopen2(file, groupname.c_str(), H5P_DEFAULT), H5Gclose);
  if (!group || !dataset_exists(group.get(), name))
    return false;

  Handle dataset(H5Dopen2(group.get(), name.c_str(), H5P_DEFAULT), H5Dclose);
  if (!dataset)
    return false;
  Handle space(H5Dget_space(dataset.get()), H5Sclose);
  if (!space || H5Sget_simple_extent_ndims(space.get()) != 1)
    return false;

  hsize_t dims[1];
  if (H5Sget_simple_extent_dims(space.get(), dims, nullptr) < 0 || !valid_size(dims[0]))
    return false;

  std::vector<T> values(static_cast<size_t>(dims[0]));
  if (!values.empty() && H5Dread(dataset.get(), datatype, H5S_ALL, H5S_ALL,
                                 H5P_DEFAULT, values.data()) < 0)
    return false;
  data = std::move(values);
  return true;
}

template<typename T>
bool read_numeric_2(hid_t file, const std::string& name,
                    const std::string& groupname, hid_t datatype,
                    std::vector<std::vector<T>>& data)
{
  if (!data.empty() || file < 0)
    return false;

  Handle group(H5Gopen2(file, groupname.c_str(), H5P_DEFAULT), H5Gclose);
  if (!group || !dataset_exists(group.get(), name))
    return false;

  Handle dataset(H5Dopen2(group.get(), name.c_str(), H5P_DEFAULT), H5Dclose);
  if (!dataset)
    return false;
  Handle space(H5Dget_space(dataset.get()), H5Sclose);
  if (!space || H5Sget_simple_extent_ndims(space.get()) != 2)
    return false;

  hsize_t dims[2];
  if (H5Sget_simple_extent_dims(space.get(), dims, nullptr) < 0 ||
      !valid_matrix_size(dims[0], dims[1]))
    return false;

  const size_t rows = static_cast<size_t>(dims[0]);
  const size_t columns = static_cast<size_t>(dims[1]);
  std::vector<T> flat(rows * columns);
  if (!flat.empty() && H5Dread(dataset.get(), datatype, H5S_ALL, H5S_ALL,
                               H5P_DEFAULT, flat.data()) < 0)
    return false;

  std::vector<std::vector<T>> values(rows, std::vector<T>(columns));
  for (size_t row = 0; row < rows; ++row)
    for (size_t column = 0; column < columns; ++column)
      values[row][column] = flat[row * columns + column];
  data = std::move(values);
  return true;
}

bool write_dataset(hid_t file, const std::string& name,
                   const std::string& groupname, hid_t datatype,
                   int rank, const hsize_t* dims, const void* values)
{
  if (file < 0)
    return false;

  Handle group(H5Gopen2(file, groupname.c_str(), H5P_DEFAULT), H5Gclose);
  if (!group)
    return false;
  Handle space(H5Screate_simple(rank, dims, nullptr), H5Sclose);
  if (!space)
    return false;
  Handle dataset(H5Dcreate2(group.get(), name.c_str(), datatype, space.get(),
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Dclose);
  return dataset && H5Dwrite(dataset.get(), datatype, H5S_ALL, H5S_ALL,
                             H5P_DEFAULT, values) >= 0;
}

template<typename T>
bool write_numeric_1(hid_t file, const std::string& name,
                     const std::string& groupname, hid_t datatype,
                     const std::vector<T>& data)
{
  if (data.empty())
    return true;
  const hsize_t dims[1] = {static_cast<hsize_t>(data.size())};
  return write_dataset(file, name, groupname, datatype, 1, dims, data.data());
}

template<typename T>
bool write_numeric_2(hid_t file, const std::string& name,
                     const std::string& groupname, hid_t datatype,
                     const std::vector<std::vector<T>>& data)
{
  if (data.empty() || data.front().empty())
    return true;

  const size_t columns = data.front().size();
  if (data.size() > std::numeric_limits<size_t>::max() / columns)
    return false;
  std::vector<T> flat;
  flat.reserve(data.size() * columns);
  for (const auto& row : data)
  {
    if (row.size() != columns)
      return false;
    flat.insert(flat.end(), row.begin(), row.end());
  }

  const hsize_t dims[2] = {static_cast<hsize_t>(data.size()),
                           static_cast<hsize_t>(columns)};
  return write_dataset(file, name, groupname, datatype, 2, dims, flat.data());
}

Handle variable_string_type()
{
  hid_t type = H5Tcopy(H5T_C_S1);
  if (type >= 0 && H5Tset_size(type, H5T_VARIABLE) < 0)
  {
    H5Tclose(type);
    type = H5I_INVALID_HID;
  }
  return Handle(type, H5Tclose);
}

bool read_strings(hid_t file, const std::string& name,
                  const std::string& groupname, int rank,
                  std::vector<std::string>& flat, hsize_t* dims)
{
  if (file < 0)
    return false;

  Handle group(H5Gopen2(file, groupname.c_str(), H5P_DEFAULT), H5Gclose);
  if (!group || !dataset_exists(group.get(), name))
    return false;
  Handle dataset(H5Dopen2(group.get(), name.c_str(), H5P_DEFAULT), H5Dclose);
  if (!dataset)
    return false;
  Handle space(H5Dget_space(dataset.get()), H5Sclose);
  if (!space || H5Sget_simple_extent_ndims(space.get()) != rank ||
      H5Sget_simple_extent_dims(space.get(), dims, nullptr) < 0)
    return false;

  if (rank == 1 && !valid_size(dims[0]))
    return false;
  if (rank == 2 && !valid_matrix_size(dims[0], dims[1]))
    return false;
  const size_t count = static_cast<size_t>(dims[0]) *
                       (rank == 2 ? static_cast<size_t>(dims[1]) : 1);
  Handle datatype = variable_string_type();
  if (!datatype)
    return false;

  std::vector<char*> values(count, nullptr);
  if (count == 0)
    return true;
  if (H5Dread(dataset.get(), datatype.get(), H5S_ALL, H5S_ALL,
              H5P_DEFAULT, values.data()) < 0)
  {
    H5Treclaim(datatype.get(), space.get(), H5P_DEFAULT, values.data());
    return false;
  }

  std::vector<std::string> strings;
  strings.reserve(count);
  for (const char* value : values)
    strings.emplace_back(value ? value : "");
  const bool reclaimed = H5Treclaim(datatype.get(), space.get(),
                                    H5P_DEFAULT, values.data()) >= 0;
  if (reclaimed)
    flat = std::move(strings);
  return reclaimed;
}

bool write_strings(hid_t file, const std::string& name,
                   const std::string& groupname, int rank,
                   const hsize_t* dims,
                   const std::vector<std::string>& flat)
{
  Handle datatype = variable_string_type();
  if (!datatype)
    return false;

  std::vector<const char*> values;
  values.reserve(flat.size());
  for (const auto& value : flat)
    values.push_back(value.c_str());
  return write_dataset(file, name, groupname, datatype.get(), rank, dims,
                       values.data());
}

herr_t collect_group(hid_t, const char* name, const H5O_info_t* info, void* data)
{
  if (name[0] != '.' && info->type == H5O_TYPE_GROUP)
    static_cast<std::vector<std::string>*>(data)->emplace_back(name);
  return 0;
}
} // namespace

HDF5Tool::HDF5Tool(std::string filename)
{
#ifdef _WIN32
  const int exists = _access(filename.c_str(), 0);
#else
  const int exists = access(filename.c_str(), F_OK);
#endif
  if (exists == 0)
    file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
  else if (errno == ENOENT)
    file = H5Fcreate(filename.c_str(), H5F_ACC_EXCL,
                     H5P_DEFAULT, H5P_DEFAULT);

  if (file < 0)
    throw std::runtime_error("Cannot open or create HDF5 file: " + filename);
}

HDF5Tool::~HDF5Tool()
{
  if (file >= 0)
    H5Fclose(file);
}

bool HDF5Tool::nameExists(std::string name)
{
  return file >= 0 && H5Lexists(file, name.c_str(), H5P_DEFAULT) > 0;
}

bool HDF5Tool::createGroup(std::string groupname)
{
  if (file < 0)
    return false;
  Handle group(H5Gcreate2(file, groupname.c_str(), H5P_DEFAULT,
                          H5P_DEFAULT, H5P_DEFAULT), H5Gclose);
  return static_cast<bool>(group);
}

bool HDF5Tool::renameGroup(std::string groupname, std::string new_groupname)
{
  return nameExists(groupname) && !nameExists(new_groupname) &&
         H5Lmove(file, groupname.c_str(), file, new_groupname.c_str(),
                 H5P_DEFAULT, H5P_DEFAULT) >= 0;
}

bool HDF5Tool::deleteGroup(std::string groupname)
{
  return nameExists(groupname) &&
         H5Ldelete(file, groupname.c_str(), H5P_DEFAULT) >= 0;
}

std::vector<std::string> HDF5Tool::getGroupsFromFile()
{
  std::vector<std::string> groups;
  if (file >= 0 && H5Ovisit(file, H5_INDEX_NAME, H5_ITER_NATIVE,
                            collect_group, &groups, H5O_INFO_BASIC) < 0)
    groups.clear();
  return groups;
}

bool HDF5Tool::deleteDataset(std::string name, std::string groupname)
{
  if (file < 0)
    return false;
  Handle group(H5Gopen2(file, groupname.c_str(), H5P_DEFAULT), H5Gclose);
  return group && dataset_exists(group.get(), name) &&
         H5Ldelete(group.get(), name.c_str(), H5P_DEFAULT) >= 0;
}

bool HDF5Tool::read_dataset_int_rank_1(std::string name, std::string groupname,
                                        std::vector<int>& data)
{
  return read_numeric_1(file, name, groupname, H5T_NATIVE_INT, data);
}

bool HDF5Tool::read_dataset_int_rank_2(std::string name, std::string groupname,
                                        std::vector<std::vector<int>>& data)
{
  return read_numeric_2(file, name, groupname, H5T_NATIVE_INT, data);
}

bool HDF5Tool::write_dataset_int_rank_1(std::string name, std::string groupname,
                                         std::vector<int> data)
{
  return write_numeric_1(file, name, groupname, H5T_NATIVE_INT, data);
}

bool HDF5Tool::write_dataset_int_rank_2(std::string name, std::string groupname,
                                         std::vector<std::vector<int>> data)
{
  return write_numeric_2(file, name, groupname, H5T_NATIVE_INT, data);
}

bool HDF5Tool::read_dataset_double_rank_1(std::string name, std::string groupname,
                                           std::vector<double>& data)
{
  return read_numeric_1(file, name, groupname, H5T_NATIVE_DOUBLE, data);
}

bool HDF5Tool::read_dataset_double_rank_2(std::string name, std::string groupname,
                                           std::vector<std::vector<double>>& data)
{
  return read_numeric_2(file, name, groupname, H5T_NATIVE_DOUBLE, data);
}

bool HDF5Tool::write_dataset_double_rank_1(std::string name, std::string groupname,
                                            std::vector<double> data)
{
  return write_numeric_1(file, name, groupname, H5T_NATIVE_DOUBLE, data);
}

bool HDF5Tool::write_dataset_double_rank_2(std::string name, std::string groupname,
                                            std::vector<std::vector<double>> data)
{
  return write_numeric_2(file, name, groupname, H5T_NATIVE_DOUBLE, data);
}

bool HDF5Tool::read_dataset_string_rank_1(std::string name, std::string groupname,
                                           std::vector<std::string>& data)
{
  if (!data.empty())
    return false;
  hsize_t dims[1];
  std::vector<std::string> values;
  if (!read_strings(file, name, groupname, 1, values, dims))
    return false;
  data = std::move(values);
  return true;
}

bool HDF5Tool::read_dataset_string_rank_2(std::string name, std::string groupname,
                                           std::vector<std::vector<std::string>>& data)
{
  if (!data.empty())
    return false;
  hsize_t dims[2];
  std::vector<std::string> flat;
  if (!read_strings(file, name, groupname, 2, flat, dims))
    return false;

  const size_t rows = static_cast<size_t>(dims[0]);
  const size_t columns = static_cast<size_t>(dims[1]);
  std::vector<std::vector<std::string>> values(rows, std::vector<std::string>(columns));
  for (size_t row = 0; row < rows; ++row)
    for (size_t column = 0; column < columns; ++column)
      values[row][column] = std::move(flat[row * columns + column]);
  data = std::move(values);
  return true;
}

bool HDF5Tool::write_dataset_string_rank_1(std::string name, std::string groupname,
                                            std::vector<std::string> data)
{
  if (data.empty())
    return true;
  const hsize_t dims[1] = {static_cast<hsize_t>(data.size())};
  return write_strings(file, name, groupname, 1, dims, data);
}

bool HDF5Tool::write_dataset_string_rank_2(std::string name, std::string groupname,
                                            std::vector<std::vector<std::string>> data)
{
  if (data.empty() || data.front().empty())
    return true;

  const size_t columns = data.front().size();
  if (data.size() > std::numeric_limits<size_t>::max() / columns)
    return false;
  std::vector<std::string> flat;
  flat.reserve(data.size() * columns);
  for (const auto& row : data)
  {
    if (row.size() != columns)
      return false;
    flat.insert(flat.end(), row.begin(), row.end());
  }

  const hsize_t dims[2] = {static_cast<hsize_t>(data.size()),
                           static_cast<hsize_t>(columns)};
  return write_strings(file, name, groupname, 2, dims, flat);
}
