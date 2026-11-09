#include "UserOptionsStore.hpp"
#include "HDF5Tool.hpp"
#include "Hdf5FileTransaction.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <exception>
#include <set>
#include <utility>

namespace
{
bool validKey(const std::string& key)
{
  if (key.empty() || key.front() == '/' || key.back() == '/')
  {
    return false;
  }
  bool separator = false;
  for (const unsigned char value : key)
  {
    if (value == '/')
    {
      if (separator)
      {
        return false;
      }
      separator = true;
    }
    else
    {
      if (!std::isalnum(value) && value != '_' && value != '-')
      {
        return false;
      }
      separator = false;
    }
  }
  return key != "schema_version" && key.compare(0, 15, "schema_version/") != 0;
}

bool validValue(const UserOption& definition, const UserOptionValue& value)
{
  bool matching = false;
  switch (definition.kind)
  {
    case UserOptionKind::Boolean:
      matching = std::holds_alternative<bool>(value);
      break;
    case UserOptionKind::Integer:
      matching = std::holds_alternative<int>(value);
      break;
    case UserOptionKind::Real:
      matching = std::holds_alternative<double>(value) && std::isfinite(std::get<double>(value));
      break;
    case UserOptionKind::Text:
    case UserOptionKind::ComponentPath:
      matching = std::holds_alternative<std::string>(value) &&
        std::get<std::string>(value).find('\0') == std::string::npos;
      break;
  }
  return matching && (!definition.validator || definition.validator(value));
}

bool validDefinitions(const std::string& workspace, const std::vector<UserOption>& definitions)
{
  if (!validKey(workspace) || workspace.find('/') != std::string::npos || definitions.empty())
  {
    return false;
  }
  std::set<std::string> keys;
  for (const auto& definition : definitions)
  {
    if (!validKey(definition.key) || !keys.insert(definition.key).second ||
        !validValue(definition, definition.defaultValue))
    {
      return false;
    }
  }
  // An option group must not also be the parent group of another option.
  for (const auto& key : keys)
  {
    for (const auto& other : keys)
    {
      if (other.compare(0, key.size() + 1, key + "/") == 0)
      {
        return false;
      }
    }
  }
  return true;
}

bool readInteger(HDF5Tool& file, const std::string& group, const std::string& name, int& value)
{
  std::vector<int> data;
  if (!file.read_dataset_int_rank_1(name, group, data) || data.size() != 1)
  {
    return false;
  }
  value = data.front();
  return true;
}

bool readValue(HDF5Tool& file, const std::string& group,
  const UserOption& definition, UserOptionValue& value)
{
  int kind = -1;
  if (!file.nameExists(group) || !readInteger(file, group, "kind", kind) ||
      kind != static_cast<int>(definition.kind))
  {
    return false;
  }
  if (definition.kind == UserOptionKind::Boolean || definition.kind == UserOptionKind::Integer)
  {
    int saved = 0;
    if (!readInteger(file, group, "value", saved))
    {
      return false;
    }
    if (definition.kind == UserOptionKind::Boolean)
    {
      if (saved != 0 && saved != 1)
      {
        return false;
      }
      value = saved != 0;
    }
    else
    {
      value = saved;
    }
  }
  else if (definition.kind == UserOptionKind::Real)
  {
    std::vector<double> saved;
    if (!file.read_dataset_double_rank_1("value", group, saved) || saved.size() != 1)
    {
      return false;
    }
    value = saved.front();
  }
  else
  {
    std::vector<std::string> saved;
    if (!file.read_dataset_string_rank_1("value", group, saved) || saved.size() != 1)
    {
      return false;
    }
    value = saved.front();
  }
  return validValue(definition, value);
}

bool writeValue(HDF5Tool& file, const std::string& group,
  const UserOption& definition, const UserOptionValue& value)
{
  // Rewrite only this declared item, retaining other workspaces and unknown keys.
  if (file.nameExists(group) && !file.deleteGroup(group))
  {
    return false;
  }
  if (!file.ensureGroup(group) ||
      !file.write_dataset_int_rank_1("kind", group, {static_cast<int>(definition.kind)}))
  {
    return false;
  }
  if (definition.kind == UserOptionKind::Boolean)
  {
    return file.write_dataset_int_rank_1("value", group, {std::get<bool>(value) ? 1 : 0});
  }
  if (definition.kind == UserOptionKind::Integer)
  {
    return file.write_dataset_int_rank_1("value", group, {std::get<int>(value)});
  }
  if (definition.kind == UserOptionKind::Real)
  {
    return file.write_dataset_double_rank_1("value", group, {std::get<double>(value)});
  }
  return file.write_dataset_string_rank_1("value", group, {std::get<std::string>(value)});
}

bool schemaVersion(HDF5Tool& file, const std::string& group, bool existing, std::string& error)
{
  if (existing)
  {
    int version = 0;
    if (!readInteger(file, group, "schema_version", version) || version != 1)
    {
      error = "Unsupported or missing user-options schema version in " + group;
      return false;
    }
    return true;
  }
  if (!file.ensureGroup(group) || !file.write_dataset_int_rank_1("schema_version", group, {1}))
  {
    error = "Cannot initialize user-options schema: " + file.last_error();
    return false;
  }
  return true;
}
}

UserOptionsStore::UserOptionsStore(FrameworkPaths paths) : paths(std::move(paths))
{
}

const std::string& UserOptionsStore::component_directory() const noexcept
{
  return paths.componentDirectory;
}

const std::string& UserOptionsStore::options_filename() const noexcept
{
  return paths.optionsFilename;
}

bool UserOptionsStore::load(const std::string& workspace, const std::vector<UserOption>& definitions,
  UserOptionValues& values, std::string& error)
{
  UserOptionValues candidate;
  if (!transact(workspace, definitions, candidate, true, error))
  {
    return false;
  }
  values.swap(candidate);
  return true;
}

bool UserOptionsStore::save(const std::string& workspace, const std::vector<UserOption>& definitions,
  const UserOptionValues& values, std::string& error)
{
  auto candidate = values;
  return transact(workspace, definitions, candidate, false, error);
}

bool UserOptionsStore::transact(const std::string& workspace, const std::vector<UserOption>& definitions,
  UserOptionValues& values, bool loading, std::string& error)
{
  error.clear();
  try
  {
    if (!validDefinitions(workspace, definitions))
    {
      error = "Invalid workspace option declaration; every unique key requires a typed, valid default.";
      return false;
    }
    if (!loading)
    {
      if (values.size() != definitions.size())
      {
        error = "Saving options requires exactly the declared keys.";
        return false;
      }
      for (const auto& definition : definitions)
      {
        const auto found = values.find(definition.key);
        if (found == values.end() || !validValue(definition, found->second))
        {
          error = "Invalid user option: " + definition.key;
          return false;
        }
      }
    }
    return Hdf5FileTransaction().update(paths.optionsFilename,
      [&](HDF5Tool& file, bool existed, std::string& error)
    {
      if (!schemaVersion(file, "/", existed, error) || !file.ensureGroup("framework"))
      {
        return false;
      }
      const std::string group = "workspaces/" + workspace;
      const bool workspaceExisted = file.nameExists(group);
      if (!schemaVersion(file, group, workspaceExisted, error))
      {
        return false;
      }
      for (const auto& definition : definitions)
      {
        const std::string item = group + "/" + definition.key;
        if (loading)
        {
          auto value = definition.defaultValue;
          if (file.nameExists(item) && !readValue(file, item, definition, value))
          {
            PRINT_WARNING("Invalid saved cubit-sim option %s/%s; using its declared default.\n",
              workspace.c_str(), definition.key.c_str());
            value = definition.defaultValue;
          }
          values.emplace(definition.key, std::move(value));
        }
        if (!writeValue(file, item, definition, values.at(definition.key)))
        {
          error = "Cannot write option " + definition.key + ": " + file.last_error();
          return false;
        }
      }
      return true;
    }, error);
  }
  catch (const H5::Exception& exception)
  {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  return false;
}
