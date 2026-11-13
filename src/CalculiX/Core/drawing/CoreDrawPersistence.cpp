#include "Core/application/CalculiXModelSchema.hpp"
#include "CoreDrawPersistence.hpp"
#include "CoreDraw.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <array>
#include <QJsonDocument>
#include <QJsonParseError>
#include <stdexcept>
#include <utility>

namespace
{
const std::string Root = CalculiXModelSchema::path("Core/Drawing");
const std::array<std::pair<const char*, DrawGroup>, 4> Groups = {{{"bcs", DrawGroup::BoundaryConditions}, {"loads", DrawGroup::Loads}, {"constraints", DrawGroup::Constraints}, {"orientations", DrawGroup::Orientations}}};
const std::array<std::pair<const char*, DrawFeature>, 15> Features = {{{"force", DrawFeature::Force}, {"pressure", DrawFeature::Pressure}, {"heatflux", DrawFeature::Heatflux}, {"gravity", DrawFeature::Gravity}, {"centrifugal", DrawFeature::Centrifugal}, {"trajectory", DrawFeature::Trajectory}, {"film", DrawFeature::Film}, {"radiation", DrawFeature::Radiation}, {"surface_traction", DrawFeature::SurfaceTraction}, {"body_heatflux", DrawFeature::BodyHeatflux}, {"displacement", DrawFeature::Displacement}, {"temperature", DrawFeature::Temperature}, {"orientation", DrawFeature::Orientation}, {"equation", DrawFeature::Equation}, {"equation_group", DrawFeature::EquationGroup}}};

void require(bool success, const std::string& message)
{
  if (!success)
  {
    throw std::runtime_error(message);
  }
}

std::string featureName(DrawFeature feature)
{
  for (const auto& entry : Features)
  {
    if (entry.second == feature)
    {
      return entry.first;
    }
  }
  throw std::runtime_error("Unknown drawing feature.");
}

DrawFeature parseFeature(const std::string& name)
{
  for (const auto& entry : Features)
  {
    if (name == entry.first)
    {
      return entry.second;
    }
  }
  throw std::runtime_error("Unknown saved drawing feature: " + name);
}

void checkDataset(HDF5Tool& file, const std::string& group, const std::string& name, H5T_class_t type, std::size_t count)
{
  const auto dataset = file.file->openDataSet(group + "/" + name);
  const auto space = dataset.getSpace();
  require(space.getSimpleExtentNdims() == 1 && dataset.getTypeClass() == type, "Invalid drawing dataset type or rank: " + name);
  hsize_t extent = 0;
  space.getSimpleExtentDims(&extent);
  require(extent == count, "Invalid drawing dataset length: " + name);
}

int readInt(HDF5Tool& file, const std::string& group, const std::string& name)
{
  checkDataset(file, group, name, H5T_INTEGER, 1);
  std::vector<int> values;
  require(file.read_dataset_int_rank_1(name, group, values) && values.size() == 1, "Cannot read drawing value: " + name);
  return values.front();
}

bool readBool(HDF5Tool& file, const std::string& group, const std::string& name)
{
  const int value = readInt(file, group, name);
  require(value == 0 || value == 1, "Invalid drawing boolean: " + name);
  return value == 1;
}

std::vector<std::string> readStrings(HDF5Tool& file, const std::string& group, const std::string& name, std::size_t count)
{
  checkDataset(file, group, name, H5T_STRING, count);
  auto dataset = file.file->openDataSet(group + "/" + name);
  const auto space = dataset.getSpace();
  H5::StrType type(H5::PredType::C_S1, H5T_VARIABLE);
  std::vector<char*> buffer(count, nullptr);
  dataset.read(buffer.data(), type);
  std::vector<std::string> values;
  try
  {
    for (const auto* value : buffer)
    {
      require(value != nullptr, "Null drawing string: " + name);
      values.emplace_back(value);
    }
  }
  catch (...)
  {
    H5Dvlen_reclaim(type.getId(), space.getId(), H5P_DEFAULT, buffer.data());
    throw;
  }
  H5Dvlen_reclaim(type.getId(), space.getId(), H5P_DEFAULT, buffer.data());
  return values;
}

void writeInt(HDF5Tool& file, const std::string& group, const std::string& name, int value)
{
  require(file.write_dataset_int_rank_1(name, group, {value}), "Cannot write drawing value: " + name);
}

DrawScope readScope(HDF5Tool& file, const std::string& group, const std::string& prefix)
{
  const std::string kind = readStrings(file, group, prefix + "kind", 1).front();
  DrawScope scope;
  if (kind == "model")
  {
    scope.kind = DrawScopeKind::Model;
  }
  else if (kind == "step")
  {
    scope.kind = DrawScopeKind::Step;
  }
  else if (kind == "custom")
  {
    scope.kind = DrawScopeKind::Custom;
  }
  else
  {
    throw std::runtime_error("Unknown saved drawing scope: " + kind);
  }
  scope.stepId = readInt(file, group, prefix + "step_id");
  return scope;
}

void writeScope(HDF5Tool& file, const std::string& group, const std::string& prefix, const DrawScope& scope)
{
  const std::string kind = scope.kind == DrawScopeKind::Step ? "step" : scope.kind == DrawScopeKind::Custom ? "custom" : "model";
  require(file.write_dataset_string_rank_1(prefix + "kind", group, {kind}), "Cannot write drawing scope.");
  writeInt(file, group, prefix + "step_id", scope.stepId);
}

std::size_t readCount(HDF5Tool& file, const std::string& group, const std::string& name)
{
  const int count = readInt(file, group, name + "_count");
  require(count >= 0 && count <= 1000000, "Invalid drawing reference count: " + name);
  return static_cast<std::size_t>(count);
}

void writeKeys(HDF5Tool& file, const std::string& group, const std::string& name, const std::vector<DrawEntityKey>& keys)
{
  require(keys.size() <= 1000000, "Too many drawing references.");
  writeInt(file, group, name + "_count", static_cast<int>(keys.size()));
  std::vector<std::string> features;
  std::vector<int> ids;
  for (const auto& key : keys)
  {
    features.push_back(featureName(key.feature));
    ids.push_back(key.id);
  }
  require(file.write_dataset_string_rank_1(name + "_features", group, features), "Cannot write drawing reference features.");
  require(file.write_dataset_int_rank_1(name + "_ids", group, ids), "Cannot write drawing reference IDs.");
}

std::vector<DrawEntityKey> readKeys(HDF5Tool& file, const std::string& group, const std::string& name)
{
  const auto count = readCount(file, group, name);
  std::vector<DrawEntityKey> keys;
  if (count == 0)
  {
    return keys;
  }
  const auto features = readStrings(file, group, name + "_features", count);
  checkDataset(file, group, name + "_ids", H5T_INTEGER, count);
  std::vector<int> ids;
  require(file.read_dataset_int_rank_1(name + "_ids", group, ids) && ids.size() == count, "Cannot read drawing reference IDs.");
  for (std::size_t index = 0; index < count; ++index)
  {
    keys.push_back({parseFeature(features[index]), ids[index]});
  }
  return keys;
}

DrawSavedState readState(HDF5Tool& file)
{
  require(readInt(file, Root, "schema_version") == 2, "Unsupported drawing schema version.");
  DrawSavedState saved;
  saved.scope = readScope(file, Root, "scope_");
  saved.overlayVisible = readBool(file, Root, "overlay_visible");
  checkDataset(file, Root, "size_factor", H5T_FLOAT, 1);
  std::vector<double> size;
  require(file.read_dataset_double_rank_1("size_factor", Root, size) && size.size() == 1, "Cannot read drawing size.");
  saved.sizeFactor = size.front();
  for (const auto& entry : Groups)
  {
    const std::string path = Root + "/Groups/" + entry.first;
    DrawSavedGroup group;
    group.group = entry.second;
    group.enabled = readBool(file, path, "enabled");
    group.initialized = readBool(file, path, "initialized");
    group.allInScope = readBool(file, path, "all_in_scope");
    group.sourceScope = readScope(file, path, "source_scope_");
    const auto count = readCount(file, path, "feature_filter");
    if (count > 0)
    {
      for (const auto& feature : readStrings(file, path, "feature_filter", count))
      {
        group.featureFilter.push_back(parseFeature(feature));
      }
    }
    group.entityFilter = readKeys(file, path, "entity_filter");
    group.entities = readKeys(file, path, "entity");
    saved.groups.push_back(std::move(group));
  }
  return saved;
}
}

bool CoreDrawPersistence::read(HDF5Tool& file, CoreDraw& drawing)
{
  // Old files and invalid optional drawing sections start from the current user defaults.
  try
  {
    drawing.resetToDefaults();
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot initialize CalculiX drawing defaults: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot initialize CalculiX drawing defaults: %s\n", error.what());
    return false;
  }
  try
  {
    if (!file.nameExists(Root))
    {
      return true;
    }
    auto saved = readState(file);
    std::string error;
    std::vector<std::string> payload;
    if (!file.read_dataset_string_rank_1("labels", Root, payload) || payload.size() != 1 || payload.front().size() > 4096)
    {
      PRINT_WARNING("Saved drawing settings ignored: invalid label settings payload.\n");
      return true;
    }
    QJsonParseError parse;
    const auto json = QJsonDocument::fromJson(QByteArray::fromStdString(payload.front()), &parse);
    if (parse.error != QJsonParseError::NoError || !json.isObject() || !DrawLabelSettings::from_json(json.object(), saved.labels, error))
    {
      PRINT_WARNING("Saved drawing settings ignored: invalid label settings.\n");
      return true;
    }
    if (!drawing.restoreState(saved, error))
    {
      PRINT_WARNING("Saved drawing settings ignored: %s\n", error.c_str());
    }
  }
  catch (const H5::Exception& error)
  {
    PRINT_WARNING("Saved drawing settings ignored: %s\n", error.getDetailMsg().c_str());
  }
  catch (const std::exception& error)
  {
    PRINT_WARNING("Saved drawing settings ignored: %s\n", error.what());
  }
  return true;
}

bool CoreDrawPersistence::write(HDF5Tool& file, const CoreDraw& drawing)
{
  try
  {
    const auto saved = drawing.savedState();
    std::string labelError;
    if (!saved.labels.validate(labelError))
    {
      PRINT_ERROR("Cannot save drawing labels: %s\n", labelError.c_str());
      return false;
    }
    require(file.createGroup(Root), "Cannot create drawing group.");
    writeScope(file, Root, "scope_", saved.scope);
    writeInt(file, Root, "overlay_visible", saved.overlayVisible);
    require(file.write_dataset_double_rank_1("size_factor", Root, {saved.sizeFactor}), "Cannot write drawing size.");
    require(file.createGroup(Root + "/Groups"), "Cannot create drawing groups.");
    for (const auto& source : saved.groups)
    {
      std::string name;
      for (const auto& entry : Groups)
      {
        if (source.group == entry.second)
        {
          name = entry.first;
        }
      }
      require(!name.empty(), "Unknown drawing group.");
      const std::string path = Root + "/Groups/" + name;
      require(file.createGroup(path), "Cannot create drawing group: " + name);
      writeInt(file, path, "enabled", source.enabled);
      writeInt(file, path, "initialized", source.initialized);
      writeInt(file, path, "all_in_scope", source.allInScope);
      writeScope(file, path, "source_scope_", source.sourceScope);
      std::vector<std::string> features;
      for (const auto feature : source.featureFilter)
      {
        features.push_back(featureName(feature));
      }
      writeInt(file, path, "feature_filter_count", static_cast<int>(features.size()));
      require(file.write_dataset_string_rank_1("feature_filter", path, features), "Cannot write drawing feature filter.");
      writeKeys(file, path, "entity_filter", source.entityFilter);
      writeKeys(file, path, "entity", source.entities);
    }
    const auto labels = QJsonDocument(saved.labels.to_json()).toJson(QJsonDocument::Compact).toStdString();
    if (!file.write_dataset_string_rank_1("labels", Root, {labels}))
    {
      PRINT_ERROR("Cannot save drawing label settings: %s\n", file.last_error().c_str());
      return false;
    }
    // Written last so a partial drawing section cannot be mistaken for a complete one.
    writeInt(file, Root, "schema_version", 2);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot save drawing settings: %s\n", error.getDetailMsg().c_str());
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot save drawing settings: %s\n", error.what());
  }
  return false;
}
