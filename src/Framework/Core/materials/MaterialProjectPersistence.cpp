#include "MaterialProjectPersistence.hpp"
#include "MaterialDefinitions.hpp"
#include "WorkspaceMaterialStore.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include <QUuid>
#include <limits>
#include <set>
#include <utility>
namespace {
bool failed(HDF5Tool& file, std::string& error) {
  error = file.last_error();
  if (error.empty()) {
    error = "Material HDF5 operation failed.";
  }
  return false;
}
bool bounded(HDF5Tool& file, const std::string& path, int rank, hsize_t maximum, std::string& error) {
  try {
    if (file.file == nullptr) {
      error = "Material HDF5 file is not open.";
      return false;
    }
    auto dataset = file.file->openDataSet(path);
    auto space = dataset.getSpace();
    if (space.getSimpleExtentNdims() != rank) {
      error = "Invalid material dataset rank: " + path;
      return false;
    }
    std::vector<hsize_t> dimensions(static_cast<std::size_t>(rank));
    space.getSimpleExtentDims(dimensions.data());
    hsize_t count = 1;
    for (hsize_t dimension : dimensions) {
      if (dimension > maximum || (dimension != 0 && count > maximum / dimension)) {
        error = "Material dataset exceeds its supported size: " + path;
        return false;
      }
      count *= dimension;
    }
    return true;
  } catch (const H5::Exception& exception) {
    error = "Cannot inspect material dataset " + path + ": " + exception.getDetailMsg();
    return false;
  }
}
bool text(HDF5Tool& file, const std::string& group, const std::string& name, std::string& value, std::string& error) {
  std::vector<std::string> data;
  if (!bounded(file, group + "/" + name, 1, 1, error)) {
    return false;
  }
  if (!file.read_dataset_string_rank_1(name, group, data)) {
    return failed(file, error);
  }
  if (data.size() != 1 || data.front().size() > 1024 * 1024) {
    error = "Invalid material text value: " + name;
    return false;
  }
  value = data.front();
  return true;
}
bool integer(HDF5Tool& file, const std::string& group, const std::string& name, int& value, std::string& error) {
  std::vector<int> data;
  if (!bounded(file, group + "/" + name, 1, 1, error)) {
    return false;
  }
  if (!file.read_dataset_int_rank_1(name, group, data)) {
    return failed(file, error);
  }
  if (data.size() != 1) {
    error = "Invalid material integer: " + name;
    return false;
  }
  value = data.front();
  return true;
}
bool optional_value(HDF5Tool& file, const std::string& group, const std::string& name, std::optional<double>& value, std::string& error) {
  if (!file.nameExists(group + "/" + name)) {
    value.reset();
    return true;
  }
  std::vector<double> data;
  if (!bounded(file, group + "/" + name, 1, 1, error)) {
    return false;
  }
  if (!file.read_dataset_double_rank_1(name, group, data)) {
    return failed(file, error);
  }
  if (data.size() != 1) {
    error = "Invalid optional material value: " + name;
    return false;
  }
  value = data.front();
  return true;
}
bool write_store(HDF5Tool& file, const std::string& group, const WorkspaceMaterialStore& store, std::string& error) {
  if (!file.ensureGroup(group) || !file.write_dataset_int_rank_1("schema_version", group, {1}) || !file.write_dataset_string_rank_1("workspace", group, {store.workspace()}) || !file.write_dataset_int_rank_1("next_id", group, {store.next_id()})) {
    return failed(file, error);
  }
  std::vector<int> ids;
  for (const auto& entry : store.all()) {
    ids.push_back(entry.first);
    if (!MaterialProjectPersistence::write_material(file, group + "/materials/" + std::to_string(entry.first), entry.second.data, error)) {
      return false;
    }
  }
  if (!file.write_dataset_int_rank_1("ids", group, ids) || !file.write_dataset_int_rank_1("count", group, {static_cast<int>(ids.size())})) {
    return failed(file, error);
  }
  return true;
}
}
MaterialProjectPersistence::MaterialProjectPersistence() = default;
MaterialProjectPersistence::~MaterialProjectPersistence() = default;
bool MaterialProjectPersistence::write_material(HDF5Tool& file, const std::string& group, const MaterialData& material, std::string& error) {
  if (!file.ensureGroup(group) || !file.write_dataset_string_rank_1("schema", group, {material.schema}) || !file.write_dataset_string_rank_1("name", group, {material.name}) || !file.write_dataset_string_rank_1("description", group, {material.description})) {
    return failed(file, error);
  }
  std::vector<std::string> cards;
  for (const auto& entry : material.cards) {
    cards.push_back(entry.first);
    const MaterialCard& card = entry.second;
    const std::string path = group + "/cards/" + entry.first;
    int state = 0;
    if (card.state == MaterialCardState::Active) {
      state = 1;
    }
    if (!file.ensureGroup(path) || !file.write_dataset_string_rank_1("model", path, {card.model}) || !file.write_dataset_int_rank_1("state", path, {state}) || !file.write_dataset_int_rank_1("row_count", path, {static_cast<int>(card.values.rows.size())})) {
      return failed(file, error);
    }
    std::vector<double> values;
    std::vector<int> offsets{0};
    for (const auto& row : card.values.rows) {
      values.insert(values.end(), row.values.begin(), row.values.end());
      offsets.push_back(static_cast<int>(values.size()));
    }
    if (!file.write_dataset_double_rank_1("values_si", path, values) || !file.write_dataset_int_rank_1("row_offsets", path, offsets)) {
      return failed(file, error);
    }
    if (!card.values.scalar.has_value() && file.nameExists(path + "/scalar_si") && !file.deleteDataset("scalar_si", path)) {
      return failed(file, error);
    }
    if (!card.values.referenceTemperature.has_value() && file.nameExists(path + "/reference_temperature_si") && !file.deleteDataset("reference_temperature_si", path)) {
      return failed(file, error);
    }
    if (card.values.scalar.has_value() && !file.write_dataset_double_rank_1("scalar_si", path, {*card.values.scalar})) {
      return failed(file, error);
    }
    if (card.values.referenceTemperature.has_value() && !file.write_dataset_double_rank_1("reference_temperature_si", path, {*card.values.referenceTemperature})) {
      return failed(file, error);
    }
  }
  if (!file.write_dataset_string_rank_1("card_ids", group, cards) || !file.write_dataset_int_rank_1("card_count", group, {static_cast<int>(cards.size())})) {
    return failed(file, error);
  }
  return true;
}
bool MaterialProjectPersistence::read_material(HDF5Tool& file, const std::string& group, const MaterialDefinitions& definitions, MaterialData& material, std::string& error) {
  MaterialData candidate;
  int cardCount = 0;
  if (!text(file, group, "schema", candidate.schema, error) || !text(file, group, "name", candidate.name, error) || !text(file, group, "description", candidate.description, error) || !integer(file, group, "card_count", cardCount, error)) {
    return false;
  }
  if (cardCount < 0 || cardCount > static_cast<int>(definitions.cards().size())) {
    error = "Invalid stored material card count.";
    return false;
  }
  std::vector<std::string> cards;
  if (cardCount > 0) {
    if (!bounded(file, group + "/card_ids", 1, definitions.cards().size(), error)) {
      return false;
    }
    if (!file.read_dataset_string_rank_1("card_ids", group, cards)) {
      return failed(file, error);
    }
  }
  if (cards.size() != static_cast<std::size_t>(cardCount)) {
    error = "Stored material card index does not match its count.";
    return false;
  }
  for (const std::string& key : cards) {
    const auto* definition = definitions.card(key);
    if (definition == nullptr || definition->key != key || candidate.cards.count(key) != 0) {
      error = "Unknown or duplicate stored material card: " + key;
      return false;
    }
    const std::string path = group + "/cards/" + key;
    MaterialCard card;
    int state = 0;
    int rowCount = 0;
    if (!text(file, path, "model", card.model, error) || !integer(file, path, "state", state, error) || !integer(file, path, "row_count", rowCount, error)) {
      return false;
    }
    if ((state != 0 && state != 1) || rowCount < 0 || rowCount > 1000000 || definitions.model(*definition, card.model) == nullptr) {
      error = "Invalid stored material card state, model, or row count.";
      return false;
    }
    if (state == 1) {
      card.state = MaterialCardState::Active;
    }
    if (rowCount > 0) {
      std::vector<int> offsets;
      std::vector<double> values;
      if (!bounded(file, path + "/row_offsets", 1, 1000001, error) || !bounded(file, path + "/values_si", 1, 22000000, error)) {
        return false;
      }
      if (!file.read_dataset_int_rank_1("row_offsets", path, offsets) || !file.read_dataset_double_rank_1("values_si", path, values)) {
        return failed(file, error);
      }
      if (offsets.size() != static_cast<std::size_t>(rowCount) + 1 || offsets.front() != 0 || offsets.back() < 0 || static_cast<std::size_t>(offsets.back()) != values.size()) {
        error = "Stored material table does not match its row count.";
        return false;
      }
      for (int row = 0; row < rowCount; ++row) {
        const int begin = offsets[static_cast<std::size_t>(row)];
        const int end = offsets[static_cast<std::size_t>(row) + 1];
        if (begin < 0 || end < begin || static_cast<std::size_t>(end) > values.size() || end - begin > 22) {
          error = "Stored material table has invalid row offsets.";
          return false;
        }
        MaterialTableRow restored;
        restored.values.assign(values.begin() + begin, values.begin() + end);
        card.values.rows.push_back(std::move(restored));
      }
    }
    if (!optional_value(file, path, "scalar_si", card.values.scalar, error) || !optional_value(file, path, "reference_temperature_si", card.values.referenceTemperature, error)) {
      return false;
    }
    candidate.cards.emplace(key, std::move(card));
  }
  if (!definitions.validate(candidate, false, error)) {
    return false;
  }
  material = std::move(candidate);
  return true;
}
bool MaterialProjectPersistence::save(HDF5Tool& file, const std::string& group, const WorkspaceMaterialStore& store, std::string& error) {
  const std::string token = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
  const std::string pending = group + "_pending_" + token;
  const std::string previous = group + "_previous_" + token;
  if (!write_store(file, pending, store, error)) {
    if (file.nameExists(pending)) {
      file.deleteGroup(pending);
    }
    return false;
  }
  const bool hadPrevious = file.nameExists(group);
  if (hadPrevious && !file.renameGroup(group, previous)) {
    const std::string savedError = file.last_error();
    file.deleteGroup(pending);
    error = savedError;
    return false;
  }
  if (!file.renameGroup(pending, group)) {
    const std::string savedError = file.last_error();
    if (hadPrevious) {
      file.renameGroup(previous, group);
    }
    file.deleteGroup(pending);
    error = savedError;
    return false;
  }
  if (hadPrevious && !file.deleteGroup(previous)) {
    return failed(file, error);
  }
  return true;
}
bool MaterialProjectPersistence::load(HDF5Tool& file, const std::string& group, WorkspaceMaterialStore& store, const MaterialDefinitions& definitions, std::string& error) {
  if (!file.nameExists(group)) {
    store.clear();
    return true;
  }
  int schema = 0;
  int count = 0;
  int nextId = 0;
  std::string workspace;
  if (!integer(file, group, "schema_version", schema, error) || !integer(file, group, "count", count, error) || !integer(file, group, "next_id", nextId, error) || !text(file, group, "workspace", workspace, error)) {
    return false;
  }
  if (schema != 1 || count < 0 || count > 1000000 || workspace != store.workspace()) {
    error = "Unsupported or invalid material workspace data.";
    return false;
  }
  std::vector<int> ids;
  if (count > 0) {
    if (!bounded(file, group + "/ids", 1, 1000000, error)) {
      return false;
    }
    if (!file.read_dataset_int_rank_1("ids", group, ids)) {
      return failed(file, error);
    }
  }
  if (ids.size() != static_cast<std::size_t>(count)) {
    error = "Material index does not match its count.";
    return false;
  }
  std::map<int, MaterialInstance> candidate;
  for (int id : ids) {
    if (id < 1 || candidate.count(id) != 0) {
      error = "Invalid or duplicate material ID.";
      return false;
    }
    MaterialInstance instance;
    instance.reference = {workspace, id};
    if (!read_material(file, group + "/materials/" + std::to_string(id), definitions, instance.data, error)) {
      return false;
    }
    candidate.emplace(id, std::move(instance));
  }
  return store.restore(std::move(candidate), nextId, error);
}
