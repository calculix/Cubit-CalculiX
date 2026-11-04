#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <filesystem>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace
{
std::string cleanPath(const std::string& path)
{
  std::string result;
  for (char value : path)
  {
    if (value != '/' || (!result.empty() && result.back() != '/'))
    {
      result.push_back(value);
    }
  }
  if (!result.empty() && result.back() == '/')
  {
    result.pop_back();
  }
  return result;
}

void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

std::vector<hsize_t> dimensions(const H5::DataSet& dataset, int rank, H5T_class_t type)
{
  const auto space = dataset.getSpace();
  require(space.getSimpleExtentNdims() == rank, "Unexpected dataset rank.");
  require(dataset.getTypeClass() == type, "Unexpected dataset type.");
  std::vector<hsize_t> shape(rank);
  space.getSimpleExtentDims(shape.data());
  return shape;
}

std::size_t elementCount(const std::vector<hsize_t>& shape)
{
  std::size_t count = 1;
  for (auto extent : shape)
  {
    require(extent <= (std::numeric_limits<std::size_t>::max)(), "Dataset extent is too large.");
    require(extent == 0 || count <= (std::numeric_limits<std::size_t>::max)() / extent, "Dataset dimensions overflow.");
    count *= static_cast<std::size_t>(extent);
  }
  return count;
}

template<class T>
void readNumeric(H5::H5File& file, const std::string& path, std::vector<T>& values, const H5::PredType& type, H5T_class_t kind)
{
  require(values.empty(), "Read destination is not empty.");
  auto dataset = file.openDataSet(path);
  const auto shape = dimensions(dataset, 1, kind);
  std::vector<T> saved(elementCount(shape));
  if (!saved.empty())
  {
    dataset.read(saved.data(), type);
  }
  values.swap(saved);
}

template<class T>
void readNumeric(H5::H5File& file, const std::string& path, std::vector<std::vector<T>>& values, const H5::PredType& type, H5T_class_t kind)
{
  require(values.empty(), "Read destination is not empty.");
  auto dataset = file.openDataSet(path);
  const auto shape = dimensions(dataset, 2, kind);
  std::vector<T> flat(elementCount(shape));
  if (!flat.empty())
  {
    dataset.read(flat.data(), type);
  }
  std::vector<std::vector<T>> saved(static_cast<std::size_t>(shape[0]));
  const auto width = static_cast<std::size_t>(shape[1]);
  for (std::size_t row = 0; row < saved.size(); ++row)
  {
    saved[row].assign(flat.begin() + row * width, flat.begin() + (row + 1) * width);
  }
  values.swap(saved);
}

template<class T>
void writeNumeric(H5::H5File& file, const std::string& path, const std::vector<T>& values, const H5::PredType& type)
{
  if (values.empty())
  {
    return;
  }
  const hsize_t shape[] = {values.size()};
  const H5::DataSpace space(1, shape);
  auto dataset = file.createDataSet(path, type, space);
  dataset.write(values.data(), type);
}

template<class T>
void writeNumeric(H5::H5File& file, const std::string& path, const std::vector<std::vector<T>>& values, const H5::PredType& type)
{
  if (values.empty())
  {
    return;
  }
  const std::size_t width = values.front().size();
  for (const auto& row : values)
  {
    require(row.size() == width, "Table rows have inconsistent widths.");
  }
  if (width == 0)
  {
    return;
  }
  const std::vector<hsize_t> shape = {values.size(), width};
  std::vector<T> flat;
  flat.reserve(elementCount(shape));
  for (const auto& row : values)
  {
    flat.insert(flat.end(), row.begin(), row.end());
  }
  const H5::DataSpace space(2, shape.data());
  auto dataset = file.createDataSet(path, type, space);
  dataset.write(flat.data(), type);
}

std::vector<std::string> readStrings(H5::H5File& file, const std::string& path, int rank, std::vector<hsize_t>& shape)
{
  auto dataset = file.openDataSet(path);
  shape = dimensions(dataset, rank, H5T_STRING);
  const auto count = elementCount(shape);
  std::vector<std::string> result;
  if (count == 0)
  {
    return result;
  }
  H5::StrType type(H5::PredType::C_S1, H5T_VARIABLE);
  const auto storedType = dataset.getStrType();
  type.setCset(storedType.getCset());
  if (storedType.isVariableStr())
  {
    std::vector<char*> buffer(count, nullptr);
    const auto space = dataset.getSpace();
    dataset.read(buffer.data(), type);
    try
    {
      for (const auto* value : buffer)
      {
        require(value != nullptr, "Null string in dataset.");
        result.emplace_back(value);
      }
    }
    catch (...)
    {
      H5Dvlen_reclaim(type.getId(), space.getId(), H5P_DEFAULT, buffer.data());
      throw;
    }
    H5Dvlen_reclaim(type.getId(), space.getId(), H5P_DEFAULT, buffer.data());
  }
  else
  {
    const std::size_t width = storedType.getSize();
    require(width > 0 && count <= (std::numeric_limits<std::size_t>::max)() / width, "String dataset dimensions overflow.");
    std::vector<char> buffer(count * width);
    dataset.read(buffer.data(), storedType);
    for (std::size_t index = 0; index < count; ++index)
    {
      const char* begin = buffer.data() + index * width;
      const char* end = std::find(begin, begin + width, '\0');
      result.emplace_back(begin, end);
    }
  }
  return result;
}

void readText(H5::H5File& file, const std::string& path, std::vector<std::string>& values)
{
  require(values.empty(), "Read destination is not empty.");
  std::vector<hsize_t> shape;
  auto saved = readStrings(file, path, 1, shape);
  values.swap(saved);
}

void readText(H5::H5File& file, const std::string& path, std::vector<std::vector<std::string>>& values)
{
  require(values.empty(), "Read destination is not empty.");
  std::vector<hsize_t> shape;
  auto flat = readStrings(file, path, 2, shape);
  std::vector<std::vector<std::string>> saved(static_cast<std::size_t>(shape[0]));
  const auto width = static_cast<std::size_t>(shape[1]);
  for (std::size_t row = 0; row < saved.size(); ++row)
  {
    saved[row].assign(flat.begin() + row * width, flat.begin() + (row + 1) * width);
  }
  values.swap(saved);
}

void writeTextFlat(H5::H5File& file, const std::string& path, const std::vector<const char*>& values, const std::vector<hsize_t>& shape)
{
  if (values.empty())
  {
    return;
  }
  H5::StrType type(H5::PredType::C_S1, H5T_VARIABLE);
  const H5::DataSpace space(static_cast<int>(shape.size()), shape.data());
  auto dataset = file.createDataSet(path, type, space);
  dataset.write(values.data(), type);
}

void writeText(H5::H5File& file, const std::string& path, const std::vector<std::string>& values)
{
  std::vector<const char*> buffer;
  for (const auto& value : values)
  {
    buffer.push_back(value.c_str());
  }
  writeTextFlat(file, path, buffer, {values.size()});
}

void writeText(H5::H5File& file, const std::string& path, const std::vector<std::vector<std::string>>& values)
{
  if (values.empty())
  {
    return;
  }
  const auto width = values.front().size();
  std::vector<const char*> buffer;
  for (const auto& row : values)
  {
    require(row.size() == width, "Table rows have inconsistent widths.");
    for (const auto& value : row)
    {
      buffer.push_back(value.c_str());
    }
  }
  writeTextFlat(file, path, buffer, {values.size(), width});
}

herr_t collectGroups(hid_t, const char* name, const H5O_info_t* info, void* data)
{
  try
  {
    if (name[0] != '.' && info->type == H5O_TYPE_GROUP)
    {
      static_cast<std::vector<std::string>*>(data)->emplace_back(name);
    }
    return 0;
  }
  catch (...)
  {
    // Never let a C++ exception cross the HDF5 C callback boundary.
    return -1;
  }
}
}

HDF5Tool::HDF5Tool(std::string filename, bool createIfMissing)
{
  H5::Exception::dontPrint();
  const bool exists = std::filesystem::exists(std::filesystem::u8path(filename));
  if (!exists && !createIfMissing)
  {
    throw std::runtime_error("The HDF5 file does not exist: " + filename);
  }
  // Exclusive creation cannot truncate an existing file after an access failure.
  file = new H5::H5File(filename, exists ? H5F_ACC_RDWR : H5F_ACC_EXCL);
}

HDF5Tool::~HDF5Tool() noexcept
{
  close();
}

bool HDF5Tool::close() noexcept
{
  auto* opened = file;
  file = nullptr;
  if (opened == nullptr)
  {
    return true;
  }
  bool success = true;
  try
  {
    opened->close();
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot close HDF5 file: %s\n", error.getDetailMsg().c_str());
    success = false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot close HDF5 file: %s\n", error.what());
    success = false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot close HDF5 file: unknown exception.\n");
    success = false;
  }
  try
  {
    delete opened;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot release HDF5 file handle.\n");
    success = false;
  }
  return success;
}

const std::string& HDF5Tool::last_error() const
{
  return lastError;
}

bool HDF5Tool::nameExists(std::string name)
{
  require(file != nullptr, "HDF5 file is not open.");
  const std::string path = cleanPath(name);
  if (path.empty())
  {
    return true;
  }
  std::size_t position = 0;
  do
  {
    position = path.find('/', position + 1);
    const std::string prefix = path.substr(0, position);
    const auto status = H5Lexists(file->getId(), prefix.c_str(), H5P_DEFAULT);
    require(status >= 0, "Cannot inspect HDF5 path.");
    if (status == 0)
    {
      return false;
    }
  }
  while (position != std::string::npos);
  return true;
}

bool HDF5Tool::createGroup(std::string groupname)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    file->createGroup(cleanPath(groupname));
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = error.what();
  }
  return false;
}

bool HDF5Tool::ensureGroup(const std::string& groupname)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    const std::string path = cleanPath(groupname);
    std::size_t position = 0;
    do
    {
      position = path.find('/', position + 1);
      const auto prefix = path.substr(0, position);
      if (!nameExists(prefix))
      {
        file->createGroup(prefix);
      }
    }
    while (position != std::string::npos);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = error.what();
  }
  return false;
}

bool HDF5Tool::renameGroup(std::string groupname, std::string new_groupname)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    if (!nameExists(groupname) || nameExists(new_groupname))
    {
      lastError = "Source group is missing or destination already exists.";
      return false;
    }
    require(H5Lmove(file->getId(), cleanPath(groupname).c_str(), file->getId(), cleanPath(new_groupname).c_str(), H5P_DEFAULT, H5P_DEFAULT) >= 0, "Cannot rename HDF5 group.");
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = error.what();
  }
  return false;
}

bool HDF5Tool::deleteGroup(std::string groupname)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    if (!nameExists(groupname))
    {
      return false;
    }
    require(H5Ldelete(file->getId(), cleanPath(groupname).c_str(), H5P_DEFAULT) >= 0, "Cannot delete HDF5 group.");
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = error.what();
  }
  return false;
}

bool HDF5Tool::deleteDataset(std::string name, std::string groupname)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    const auto path = cleanPath(groupname + "/" + name);
    if (!nameExists(path))
    {
      return false;
    }
    require(H5Ldelete(file->getId(), path.c_str(), H5P_DEFAULT) >= 0, "Cannot delete HDF5 dataset.");
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = error.what();
  }
  return false;
}

std::vector<std::string> HDF5Tool::getGroupsFromFile()
{
  std::vector<std::string> groups;
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    require(H5Ovisit(file->getId(), H5_INDEX_NAME, H5_ITER_NATIVE, collectGroups, &groups, H5O_INFO_BASIC) >= 0, "Cannot enumerate HDF5 groups.");
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot enumerate HDF5 groups: %s\n", error.getDetailMsg().c_str());
    groups.clear();
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot enumerate HDF5 groups: %s\n", error.what());
    groups.clear();
  }
  return groups;
}

bool HDF5Tool::read_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> &data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    readNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_INT, H5T_INTEGER);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::write_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    writeNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_INT);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::read_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> &data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    readNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_INT, H5T_INTEGER);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::write_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    writeNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_INT);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::read_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> &data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    readNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_DOUBLE, H5T_FLOAT);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::write_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    writeNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_DOUBLE);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::read_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> &data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    readNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_DOUBLE, H5T_FLOAT);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::write_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    writeNumeric(*file, cleanPath(groupname + "/" + name), data, H5::PredType::NATIVE_DOUBLE);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::read_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> &data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    readText(*file, cleanPath(groupname + "/" + name), data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::write_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    writeText(*file, cleanPath(groupname + "/" + name), data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::read_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> &data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    readText(*file, cleanPath(groupname + "/" + name), data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}

bool HDF5Tool::write_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> data)
{
  lastError.clear();
  try
  {
    require(file != nullptr, "HDF5 file is not open.");
    writeText(*file, cleanPath(groupname + "/" + name), data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.getDetailMsg();
  }
  catch (const std::exception& error)
  {
    lastError = cleanPath(groupname + "/" + name) + ": " + error.what();
  }
  return false;
}
