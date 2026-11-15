#include "CoreFrameworkMaterialLibrary.hpp"
#include "CoreFrameworkMaterials.hpp"
#include "FrameworkMaterialsInterface.hpp"
#include "MaterialProjectPersistence.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include <exception>
#include "Framework/Storage/HDF5Tool.hpp"
#include "Framework/Storage/Hdf5FileTransaction.hpp"
#include "Framework/Utility/FrameworkPaths.hpp"
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <cctype>
#include <set>
#include <utility>
namespace {
const std::string libraryRoot = "cubit-sim/material-library";
std::vector<UserOption> library_options() {
  return {{"library/file", UserOptionKind::ComponentPath, std::string("libmaterial.hdf5"), [](const UserOptionValue& value) { return !std::get<std::string>(value).empty(); }}};
}

int libraryAnchor = 0;
std::string group_name(std::string value) {
  while (!value.empty() && value.front() == '/') {
    value.erase(value.begin());
  }
  while (!value.empty() && value.back() == '/') {
    value.pop_back();
  }
  return value;
}
bool valid_name(const std::string& name) {
  if (name.empty() || name.size() > 255) {
    return false;
  }
  for (unsigned char character : name) {
    if (character < 32 || character == 127) {
      return false;
    }
  }
  return true;
}
std::string entry_key(const std::string& workspace, const std::string& group, const std::string& name) {
  return workspace + "\x1f" + group_name(group) + "\x1f" + name;
}
bool failure(HDF5Tool& file, std::string& error) {
  error = file.last_error();
  if (error.empty()) {
    error = "Material library HDF5 operation failed.";
  }
  return false;
}
bool count(HDF5Tool& file, const std::string& name, int& result, std::string& error) {
  std::vector<int> values;
  if (!file.read_dataset_int_rank_1(name, libraryRoot, values)) {
    return failure(file, error);
  }
  if (values.size() != 1 || values.front() < 0 || values.front() > 100000) {
    error = "Invalid material library count: " + name;
    return false;
  }
  result = values.front();
  return true;
}
}
CoreFrameworkMaterialLibrary::CoreFrameworkMaterialLibrary(const MaterialDefinitions& catalog) : definitions(catalog) {
}
CoreFrameworkMaterialLibrary::~CoreFrameworkMaterialLibrary() = default;
bool CoreFrameworkMaterialLibrary::set_filename(const std::string& filename, std::string& error) {
  if (filename.empty()) {
    error = "Material library filename is empty.";
    return false;
  }
  libraryFilename = QFileInfo(QString::fromStdString(filename)).absoluteFilePath().toStdString();
  return true;
}
const std::string& CoreFrameworkMaterialLibrary::filename() const noexcept {
  return libraryFilename;
}
bool CoreFrameworkMaterialLibrary::initialize(std::string& error) {
  return mutate([](Groups&, Entries&, std::string&) {
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::initialize(UserOptionsInterface& store, std::string& error) {
  options = nullptr;
  UserOptionValues values;
  if (!store.load("framework", library_options(), values, error)) {
    return false;
  }
  const std::string path = QDir(QString::fromStdString(store.component_directory())).absoluteFilePath(QString::fromStdString(std::get<std::string>(values.at("library/file")))).toStdString();
  if (!set_filename(path, error) || !initialize(error)) {
    return false;
  }
  options = &store;
  return true;
}
bool CoreFrameworkMaterialLibrary::set_file_option(const std::string& filename, std::string& error) {
  if (options == nullptr || filename.empty()) {
    error = "The Framework material library preference service is unavailable or its filename is empty.";
    return false;
  }
  const std::string path = QDir(QString::fromStdString(options->component_directory())).absoluteFilePath(QString::fromStdString(filename)).toStdString();
  std::string previous = libraryFilename;
  try {
    if (set_filename(path, error) && initialize(error) && options->save("framework", library_options(), {{"library/file", path}}, error)) {
      return true;
    }
  } catch (const std::exception& exception) {
    error = exception.what();
  } catch (...) {
    error = "Cannot change the shared material library file.";
  }
  libraryFilename.swap(previous);
  return false;
}
void CoreFrameworkMaterialLibrary::shutdown() noexcept {
  options = nullptr;
}
bool CoreFrameworkMaterialLibrary::read(HDF5Tool& file, Groups& groups, Entries& entries, std::string& error) const {
  groups.clear();
  entries.clear();
  if (file.file == nullptr) {
    return failure(file, error);
  }
  if (!file.nameExists(libraryRoot)) {
    if (!file.getGroupsFromFile().empty()) {
      error = "Unsupported material library format. Import a Framework material library.";
      return false;
    }
    return true;
  }
  int schema = 0;
  int groupCount = 0;
  int entryCount = 0;
  if (!count(file, "schema_version", schema, error) || !count(file, "group_count", groupCount, error) || !count(file, "material_count", entryCount, error)) {
    return false;
  }
  if (schema != 1) {
    error = "Unsupported material library schema.";
    return false;
  }
  for (int index = 0; index < groupCount; ++index) {
    std::vector<std::string> record;
    if (!file.read_dataset_string_rank_1("record", libraryRoot + "/groups/" + std::to_string(index), record)) {
      return failure(file, error);
    }
    if (record.size() != 2 || !valid_name(record[0]) || record[1].size() > 1024 * 1024 || !groups.emplace(record[0], record[1]).second) {
      error = "Invalid or duplicate material library group.";
      return false;
    }
  }
  for (int index = 0; index < entryCount; ++index) {
    MaterialLibraryEntry entry;
    const std::string path = libraryRoot + "/materials/" + std::to_string(index);
    std::vector<std::string> group;
    if (!file.read_dataset_string_rank_1("group", path, group)) {
      return failure(file, error);
    }
    if (group.size() != 1 || groups.count(group.front()) == 0) {
      error = "Material library entry references an unknown group.";
      return false;
    }
    entry.group = group.front();
    if (!MaterialProjectPersistence::read_material(file, path + "/data", definitions, entry.data, error)) {
      return false;
    }
    if (!entries.emplace(entry_key(entry.data.schema, entry.group, entry.data.name), std::move(entry)).second) {
      error = "Duplicate material library entry.";
      return false;
    }
  }
  return true;
}
bool CoreFrameworkMaterialLibrary::write(HDF5Tool& file, const Groups& groups, const Entries& entries, std::string& error) const {
  if (file.nameExists(libraryRoot) && !file.deleteGroup(libraryRoot)) {
    return failure(file, error);
  }
  if (!file.ensureGroup(libraryRoot) || !file.write_dataset_int_rank_1("schema_version", libraryRoot, {1}) || !file.write_dataset_int_rank_1("group_count", libraryRoot, {static_cast<int>(groups.size())}) || !file.write_dataset_int_rank_1("material_count", libraryRoot, {static_cast<int>(entries.size())})) {
    return failure(file, error);
  }
  int index = 0;
  for (const auto& group : groups) {
    const std::string path = libraryRoot + "/groups/" + std::to_string(index);
    if (!file.ensureGroup(path) || !file.write_dataset_string_rank_1("record", path, {group.first, group.second})) {
      return failure(file, error);
    }
    ++index;
  }
  index = 0;
  for (const auto& item : entries) {
    const std::string path = libraryRoot + "/materials/" + std::to_string(index);
    if (!file.ensureGroup(path) || !file.write_dataset_string_rank_1("group", path, {item.second.group})) {
      return failure(file, error);
    }
    if (!MaterialProjectPersistence::write_material(file, path + "/data", item.second.data, error)) {
      return false;
    }
    ++index;
  }
  return true;
}
bool CoreFrameworkMaterialLibrary::mutate(const Mutation& mutation, std::string& error) {
  if (libraryFilename.empty()) {
    FrameworkPaths paths;
    if (!FrameworkPaths::resolve(&libraryAnchor, paths, error)) {
      return false;
    }
    libraryFilename = QDir(QString::fromStdString(paths.componentDirectory)).filePath("libmaterial.hdf5").toStdString();
  }
  Hdf5FileTransaction transaction;
  return transaction.update(libraryFilename, [&](HDF5Tool& file, bool, std::string& problem) {
    Groups groups;
    Entries entries;
    if (!read(file, groups, entries, problem) || !mutation(groups, entries, problem)) {
      return false;
    }
    return write(file, groups, entries, problem);
  }, error);
}
bool CoreFrameworkMaterialLibrary::create_group(const std::string& name, std::string& error) {
  const std::string group = group_name(name);
  return mutate([&](Groups& groups, Entries&, std::string& problem) {
    if (!valid_name(group) || groups.count(group) != 0) {
      problem = "Material library group is invalid or already exists: " + group;
      return false;
    }
    std::size_t slash = group.find('/');
    while (slash != std::string::npos) {
      groups.emplace(group.substr(0, slash), "");
      slash = group.find('/', slash + 1);
    }
    groups.emplace(group, "");
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::modify_group(const std::string& name, const std::string& data, int mode, std::string& error) {
  const std::string group = group_name(name);
  return mutate([&](Groups& groups, Entries& entries, std::string& problem) {
    auto found = groups.find(group);
    if (found == groups.end()) {
      problem = "Material library group does not exist: " + group;
      return false;
    }
    if (mode == 1) {
      found->second = data;
      return true;
    }
    const std::string replacement = group_name(data);
    if (mode != 0 || !valid_name(replacement) || groups.count(replacement) != 0) {
      problem = "Invalid material library group rename.";
      return false;
    }
    Groups renamedGroups;
    for (const auto& existing : groups) {
      std::string target = existing.first;
      if (target == group || target.find(group + "/") == 0) {
        target = replacement + target.substr(group.size());
      }
      if (!renamedGroups.emplace(target, existing.second).second) {
        problem = "Material library group rename would merge existing groups.";
        return false;
      }
    }
    Entries renamedEntries;
    for (const auto& existing : entries) {
      MaterialLibraryEntry entry = existing.second;
      if (entry.group == group || entry.group.find(group + "/") == 0) {
        entry.group = replacement + entry.group.substr(group.size());
      }
      const std::string key = entry_key(entry.data.schema, entry.group, entry.data.name);
      if (!renamedEntries.emplace(key, std::move(entry)).second) {
        problem = "Material library group rename would merge existing materials.";
        return false;
      }
    }
    groups = std::move(renamedGroups);
    entries = std::move(renamedEntries);
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::delete_group(const std::string& name, std::string& error) {
  const std::string group = group_name(name);
  return mutate([&](Groups& groups, Entries& entries, std::string& problem) {
    if (groups.count(group) == 0) {
      problem = "Material library group does not exist: " + group;
      return false;
    }
    for (const auto& entry : entries) {
      if (entry.second.group == group || entry.second.group.find(group + "/") == 0) {
        problem = "Delete materials from the group before deleting the group.";
        return false;
      }
    }
    for (auto iterator = groups.begin(); iterator != groups.end();) {
      if (iterator->first == group || iterator->first.find(group + "/") == 0) {
        iterator = groups.erase(iterator);
      } else {
        ++iterator;
      }
    }
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::create_material(const std::string& workspace, const std::string& name, const std::string& groupName, std::string& error) {
  const std::string group = group_name(groupName);
  return mutate([&](Groups& groups, Entries& entries, std::string& problem) {
    const std::string key = entry_key(workspace, group, name);
    MaterialData data;
    data.schema = workspace;
    data.name = name;
    if (groups.count(group) == 0 || entries.count(key) != 0 || !definitions.validate(data, false, problem)) {
      if (problem.empty()) {
        problem = "Material library group is missing or material already exists.";
      }
      return false;
    }
    entries.emplace(key, MaterialLibraryEntry{group, std::move(data)});
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::replace_material(const std::string& workspace, const std::string& name, const std::string& groupName, MaterialData data, std::string& error) {
  const std::string group = group_name(groupName);
  return mutate([&](Groups&, Entries& entries, std::string& problem) {
    const std::string oldKey = entry_key(workspace, group, name);
    auto existing = entries.find(oldKey);
    if (existing == entries.end() || data.schema != workspace || !definitions.validate(data, false, problem)) {
      if (problem.empty()) {
        problem = "Material library entry does not exist or belongs to another workspace.";
      }
      return false;
    }
    const std::string newKey = entry_key(workspace, group, data.name);
    if (newKey != oldKey && entries.count(newKey) != 0) {
      problem = "Material library name already exists.";
      return false;
    }
    entries.erase(existing);
    entries.emplace(newKey, MaterialLibraryEntry{group, std::move(data)});
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::modify_material(const std::string& workspace, const std::string& name, const std::string& groupName, const std::string& data, int mode, const std::vector<double>& values, std::string& error) {
  const std::string group = group_name(groupName);
  return mutate([&](Groups&, Entries& entries, std::string& problem) {
    const std::string key = entry_key(workspace, group, name);
    auto found = entries.find(key);
    if (found == entries.end()) {
      problem = "Material library entry does not exist.";
      return false;
    }
    MaterialData candidate = found->second.data;
    if (mode == 0) {
      candidate.name = data;
    } else if (mode == 1) {
      candidate.description = data;
    } else if (mode == 2) {
      if (values.size() != 1) {
        problem = "Material scalar requires exactly one value.";
        return false;
      }
      FrameworkMaterialsInterface bridge(workspace);
      if (!bridge.write_scalar(candidate, data, values.front(), problem)) {
        return false;
      }
    } else if (mode == 3) {
      FrameworkMaterialsInterface bridge(workspace);
      const int width = bridge.get_property_size(data);
      if (width < 1 || values.size() % static_cast<std::size_t>(width) != 0) {
        problem = "Material table has an incorrect column count.";
        return false;
      }
      std::vector<std::vector<double>> rows;
      for (std::size_t offset = 0; offset < values.size(); offset += static_cast<std::size_t>(width)) {
        rows.emplace_back(values.begin() + offset, values.begin() + offset + width);
      }
      if (!bridge.write_table(candidate, data, rows, problem)) {
        return false;
      }
    } else {
      problem = "Unsupported material library edit.";
      return false;
    }
    if (!definitions.validate(candidate, false, problem)) {
      return false;
    }
    const std::string replacement = entry_key(workspace, group, candidate.name);
    if (replacement != key && entries.count(replacement) != 0) {
      problem = "Material library name already exists.";
      return false;
    }
    entries.erase(found);
    entries.emplace(replacement, MaterialLibraryEntry{group, std::move(candidate)});
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::delete_material(const std::string& workspace, const std::string& name, const std::string& groupName, std::string& error) {
  return mutate([&](Groups&, Entries& entries, std::string& problem) {
    if (entries.erase(entry_key(workspace, groupName, name)) == 0) {
      problem = "Material library entry does not exist.";
      return false;
    }
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::read_material(const std::string& workspace, const std::string& name, const std::string& group, MaterialData& material, std::string& error) const {
  if (libraryFilename.empty()) {
    error = "Material library is not initialized.";
    return false;
  }
  HDF5Tool file(libraryFilename, false);
  Groups groups;
  Entries entries;
  if (!read(file, groups, entries, error)) {
    return false;
  }
  if (!file.close()) {
    return failure(file, error);
  }
  auto found = entries.find(entry_key(workspace, group, name));
  if (found == entries.end()) {
    error = "Material library entry does not exist in this workspace.";
    return false;
  }
  material = found->second.data;
  return true;
}
bool CoreFrameworkMaterialLibrary::export_material(const std::string& workspace, int id, const std::string& name, const std::string& groupName, std::string& error) {
  const auto* material = framework_materials().workspace(workspace).find({workspace, id});
  if (material == nullptr) {
    error = "Material does not exist in this workspace.";
    return false;
  }
  MaterialData candidate = material->data;
  candidate.name = name;
  const std::string group = group_name(groupName);
  return mutate([&](Groups& groups, Entries& entries, std::string& problem) {
    const std::string key = entry_key(workspace, group, name);
    if (groups.count(group) == 0 || entries.count(key) != 0 || !definitions.validate(candidate, false, problem)) {
      if (problem.empty()) {
        problem = "Material library group is missing or material already exists.";
      }
      return false;
    }
    entries.emplace(key, MaterialLibraryEntry{group, candidate});
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::import_material(const std::string& workspace, const std::string& name, const std::string& group, const std::string& localName, int& id, std::string& error) {
  MaterialData material;
  if (!read_material(workspace, name, group, material, error)) {
    return false;
  }
  material.name = localName;
  MaterialRef reference;
  if (!framework_materials().workspace(workspace).create(std::move(material), 0, reference, error)) {
    return false;
  }
  id = reference.id;
  return true;
}
bool CoreFrameworkMaterialLibrary::import_library(const std::string& sourceFilename, std::string& error) {
  HDF5Tool source(sourceFilename, false);
  Groups sourceGroups;
  Entries sourceEntries;
  if (!read(source, sourceGroups, sourceEntries, error)) {
    return false;
  }
  if (!source.close()) {
    return failure(source, error);
  }
  return mutate([&](Groups& groups, Entries& entries, std::string& problem) {
    for (const auto& entry : sourceEntries) {
      if (entries.count(entry.first) != 0) {
        problem = "Imported material library contains an existing material: " + entry.second.data.name;
        return false;
      }
    }
    for (const auto& group : sourceGroups) {
      groups.emplace(group);
    }
    for (const auto& entry : sourceEntries) {
      entries.emplace(entry);
    }
    return true;
  }, error);
}
bool CoreFrameworkMaterialLibrary::tree_data(const std::string& workspace, std::vector<std::vector<std::string>>& data, std::string& error) const {
  data.clear();
  if (libraryFilename.empty()) {
    error = "Material library is not initialized.";
    return false;
  }
  HDF5Tool file(libraryFilename, false);
  Groups groups;
  Entries entries;
  if (!read(file, groups, entries, error)) {
    return false;
  }
  if (!file.close()) {
    return failure(file, error);
  }
  for (const auto& group : groups) {
    std::string name = group.first;
    const auto slash = name.find_last_of('/');
    if (slash != std::string::npos) {
      name = name.substr(slash + 1);
    }
    data.push_back({name, group.second, "/" + group.first, "Group"});
  }
  for (const auto& entry : entries) {
    if (entry.second.data.schema == workspace) {
      data.push_back({entry.second.data.name, entry.second.data.description, "/" + entry.second.group + "/" + entry.second.data.name, "Material"});
    }
  }
  return true;
}
