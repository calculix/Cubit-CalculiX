#include "CoreFrameworkUnitsPersistence.hpp"
#include "CoreFrameworkUnits.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include <exception>
#include <utility>

namespace {
const std::string root = "cubit-sim/framework/units";
bool read_string(HDF5Tool& file, const std::string& group, const std::string& key, std::string& output, std::string& error) {
  std::vector<std::string> values;
  if (!file.read_dataset_string_rank_1(key, group, values) || values.size() != 1 || values.front().size() > 256) {
    error = "Invalid stored unit setting: " + group + "/" + key;
    return false;
  }
  output = values.front();
  return true;
}
}

SimUnits::CoreFrameworkUnitsPersistence::CoreFrameworkUnitsPersistence() = default;
bool SimUnits::CoreFrameworkUnitsPersistence::read(HDF5Tool& file, CoreFrameworkUnits& units, std::string& error) const {
  error.clear();
  try {
    if (file.file == nullptr) {
      error = file.last_error();
      return false;
    }
    std::string effective = root;
    if (!file.nameExists(effective) && file.nameExists(root + "-previous")) {
      effective = root + "-previous";
    }
    if (!file.nameExists(effective)) {
      units.reset_project();
      return true;
    }
    std::vector<int> schema;
    std::vector<std::string> ids;
    std::string geometry;
    if (!file.read_dataset_int_rank_1("schema_version", effective, schema) || schema != std::vector<int>{2} || !file.read_dataset_string_rank_1("workspace_ids", effective, ids) || ids.size() > units.workspace_ids().size() || !read_string(file, effective, "geometry_length_unit", geometry, error)) {
      if (error.empty()) {
        error = "Invalid or unsupported framework units schema.";
      }
      return false;
    }
    std::vector<WorkspaceUnitContext> contexts;
    for (const auto& id : ids) {
      WorkspaceUnitContext context;
      if (!units.workspace_context(id, context, error)) {
        return false;
      }
      const std::string group = effective + "/workspaces/" + id;
      if (!read_string(file, group, "system", context.system, error) || !read_string(file, group, "native_system", context.native_system, error)) {
        return false;
      }
      std::vector<std::string> quantities;
      std::vector<std::string> expressions;
      if (!file.read_dataset_string_rank_1("quantities", group, quantities) || !file.read_dataset_string_rank_1("display_units", group, expressions) || quantities.size() != expressions.size() || quantities.size() > units.quantity_ids().size()) {
        error = "Invalid stored unit display preferences.";
        return false;
      }
      context.display_units.clear();
      for (std::size_t index = 0; index < quantities.size(); ++index) {
        if (!context.display_units.emplace(quantities[index], expressions[index]).second) {
          error = "Duplicate saved quantity preference.";
          return false;
        }
      }
      contexts.push_back(std::move(context));
    }
    return units.restore_project(contexts, geometry, error);
  } catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  } catch (const std::exception& exception) {
    error = exception.what();
  }
  return false;
}
bool SimUnits::CoreFrameworkUnitsPersistence::write(HDF5Tool& file, const CoreFrameworkUnits& units, std::string& error) const {
  error.clear();
  try {
    if (file.file == nullptr) {
      error = file.last_error();
      return false;
    }
    const std::string staged = root + "-staged";
    const std::string previous = root + "-previous";
    if (!file.nameExists(root) && file.nameExists(previous) && !file.renameGroup(previous, root)) {
      error = "Cannot recover previously saved unit contexts.";
      return false;
    }
    if ((file.nameExists(staged) && !file.deleteGroup(staged)) || !file.ensureGroup(staged) || !file.write_dataset_int_rank_1("schema_version", staged, {2}) || !file.write_dataset_string_rank_1("workspace_ids", staged, units.workspace_ids()) || !file.write_dataset_string_rank_1("geometry_length_unit", staged, {units.geometry_unit()})) {
      error = "Cannot stage framework unit contexts: " + file.last_error();
      return false;
    }
    for (const auto& id : units.workspace_ids()) {
      WorkspaceUnitContext context;
      if (!units.workspace_context(id, context, error)) {
        return false;
      }
      const std::string group = staged + "/workspaces/" + id;
      std::vector<std::string> quantities;
      std::vector<std::string> expressions;
      for (const auto& preference : context.display_units) {
        quantities.push_back(preference.first);
        expressions.push_back(preference.second);
      }
      if (!file.ensureGroup(group) || !file.write_dataset_string_rank_1("system", group, {context.system}) || !file.write_dataset_string_rank_1("native_system", group, {context.native_system}) || !file.write_dataset_string_rank_1("quantities", group, quantities) || !file.write_dataset_string_rank_1("display_units", group, expressions)) {
        error = "Cannot stage workspace units: " + id + ": " + file.last_error();
        return false;
      }
    }
    file.file->flush(H5F_SCOPE_GLOBAL);
    if (file.nameExists(previous) && !file.deleteGroup(previous)) {
      error = "Cannot remove previous unit-context staging.";
      return false;
    }
    const bool hadPrevious = file.nameExists(root);
    if (hadPrevious && !file.renameGroup(root, previous)) {
      error = "Cannot preserve current unit contexts.";
      return false;
    }
    if (!file.renameGroup(staged, root)) {
      if (hadPrevious) {
        file.renameGroup(previous, root);
      }
      error = "Cannot publish staged unit contexts.";
      return false;
    }
    file.file->flush(H5F_SCOPE_GLOBAL);
    if (hadPrevious && !file.deleteGroup(previous)) {
      error = "Unit contexts saved, but previous staging could not be removed.";
      return false;
    }
    return true;
  } catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  } catch (const std::exception& exception) {
    error = exception.what();
  }
  return false;
}
bool SimUnits::CoreFrameworkUnitsPersistence::read(const std::string& filename, CoreFrameworkUnits& units, std::string& error) const {
  try {
    HDF5Tool file(filename, false);
    CoreFrameworkUnits candidate = units;
    if (!read(file, candidate, error)) {
      return false;
    }
    if (!file.close()) {
      error = file.last_error();
      return false;
    }
    units.swap_project_state(candidate);
    return true;
  } catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  } catch (const std::exception& exception) {
    error = exception.what();
  }
  return false;
}
bool SimUnits::CoreFrameworkUnitsPersistence::write(const std::string& filename, const CoreFrameworkUnits& units, std::string& error) const {
  try {
    HDF5Tool file(filename, false);
    if (!write(file, units, error)) {
      return false;
    }
    if (!file.close()) {
      error = file.last_error();
      return false;
    }
    return true;
  } catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  } catch (const std::exception& exception) {
    error = exception.what();
  }
  return false;
}
