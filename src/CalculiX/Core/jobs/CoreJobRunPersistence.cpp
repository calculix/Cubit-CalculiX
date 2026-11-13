#include "CoreJobRunPersistence.hpp"
#include "CoreJobSnapshotBuilder.hpp"
#include "Core/application/CalculiXModelSchema.hpp"
#include "Framework/Reporting/ReportJson.hpp"
#include "HDF5Tool.hpp"
#include <QJsonDocument>
#include <QJsonParseError>
#include <algorithm>
#include <exception>
#include <set>

namespace
{
const auto Root = CalculiXModelSchema::path("Jobs/RunSnapshots");

bool writeBytes(HDF5Tool& file, const std::string& path, const std::string& name, const QByteArray& bytes)
{
  if (!file.ensureGroup(path))
  {
    return false;
  }
  const hsize_t length = bytes.size();
  const H5::DataSpace space(1, &length);
  auto dataset = file.file->createDataSet(path + "/" + name, H5::PredType::NATIVE_UCHAR, space);
  if (!bytes.isEmpty())
  {
    dataset.write(bytes.constData(), H5::PredType::NATIVE_UCHAR);
  }
  return true;
}

bool readBytes(HDF5Tool& file, const std::string& path, const std::string& name, qint64 limit, QByteArray& output)
{
  const auto dataset = file.file->openDataSet(path + "/" + name);
  const auto space = dataset.getSpace();
  hsize_t length = 0;
  if (space.getSimpleExtentNdims() != 1 || dataset.getTypeClass() != H5T_INTEGER || dataset.getDataType().getSize() != 1)
  {
    return false;
  }
  space.getSimpleExtentDims(&length);
  if (limit < 0 || length > static_cast<hsize_t>(limit))
  {
    return false;
  }
  QByteArray candidate;
  candidate.resize(static_cast<int>(length));
  if (length > 0)
  {
    dataset.read(candidate.data(), H5::PredType::NATIVE_UCHAR);
  }
  output = std::move(candidate);
  return true;
}

bool writeJson(HDF5Tool& file, const std::string& path, const QJsonObject& object)
{
  const auto bytes = QJsonDocument(object).toJson(QJsonDocument::Compact);
  return bytes.size() <= 16 * 1024 * 1024 && writeBytes(file, path, "metadata", bytes);
}

bool readJson(HDF5Tool& file, const std::string& path, QJsonObject& output)
{
  QByteArray bytes;
  QJsonParseError error;
  if (!readBytes(file, path, "metadata", 16 * 1024 * 1024, bytes))
  {
    return false;
  }
  const auto document = QJsonDocument::fromJson(bytes, &error);
  if (error.error != QJsonParseError::NoError || !document.isObject())
  {
    return false;
  }
  output = document.object();
  return true;
}

bool numericShape(HDF5Tool& file, const std::string& path, const char* name, H5T_class_t type, hsize_t limit, hsize_t& count)
{
  const auto dataset = file.file->openDataSet(path + "/" + name);
  const auto space = dataset.getSpace();
  if (space.getSimpleExtentNdims() != 1 || dataset.getTypeClass() != type)
  {
    return false;
  }
  space.getSimpleExtentDims(&count);
  return count <= limit;
}

bool writeGeometry(HDF5Tool& file, const std::string& path, const DrawingGeometry& geometry)
{
  std::vector<double> points;
  std::vector<int> offsets = {0}, kinds;
  std::vector<int> annotationIds;
  std::map<std::pair<std::string, std::string>, int> palette;
  QJsonArray annotations;
  for (const auto& primitive : geometry.primitives)
  {
    kinds.push_back(static_cast<int>(primitive.kind));
    for (const auto& point : primitive.points)
    {
      points.insert(points.end(), point.begin(), point.end());
    }
    offsets.push_back(static_cast<int>(points.size()));
    const auto inserted = palette.emplace(std::make_pair(primitive.color, primitive.text), static_cast<int>(palette.size()));
    annotationIds.push_back(inserted.first->second);
    if (inserted.second)
    {
      annotations.append(QJsonObject{{"color", QString::fromStdString(primitive.color)}, {"text", QString::fromStdString(primitive.text)}});
    }
  }
  return writeJson(file, path, {{"annotations", annotations}}) &&
    file.write_dataset_double_rank_1("points", path, points) && file.write_dataset_int_rank_1("offsets", path, offsets) &&
    file.write_dataset_int_rank_1("kinds", path, kinds) && file.write_dataset_int_rank_1("annotation_ids", path, annotationIds);
}

bool readGeometry(HDF5Tool& file, const std::string& path, DrawingGeometry& output, hsize_t& budget, std::string& error)
{
  hsize_t count = 0, offsetsCount = 0, pointCount = 0, annotationCount = 0;
  if (!numericShape(file, path, "kinds", H5T_INTEGER, 1000000, count) ||
      !numericShape(file, path, "offsets", H5T_INTEGER, 1000001, offsetsCount) || offsetsCount != count + 1 ||
      !numericShape(file, path, "points", H5T_FLOAT, budget, pointCount) ||
      !numericShape(file, path, "annotation_ids", H5T_INTEGER, 1000000, annotationCount) || annotationCount != count)
  {
    return false;
  }
  budget -= pointCount;
  std::vector<int> kinds, offsets, annotationIds;
  std::vector<double> points;
  QJsonObject metadata;
  if (!file.read_dataset_int_rank_1("kinds", path, kinds) || !file.read_dataset_int_rank_1("offsets", path, offsets) ||
      !file.read_dataset_double_rank_1("points", path, points) || !file.read_dataset_int_rank_1("annotation_ids", path, annotationIds) || !readJson(file, path, metadata) ||
      !metadata.value("annotations").isArray() || metadata.value("annotations").toArray().size() > static_cast<int>(count) ||
      offsets.empty() || offsets.front() != 0 || offsets.back() != static_cast<int>(points.size()))
  {
    return false;
  }
  DrawingGeometry candidate;
  const auto annotations = metadata.value("annotations").toArray();
  for (std::size_t index = 0; index < kinds.size(); ++index)
  {
    if (kinds[index] < 0 || kinds[index] > 3 || offsets[index] < 0 || offsets[index + 1] < offsets[index] ||
        offsets[index + 1] > static_cast<int>(points.size()) || offsets[index] % 3 != 0 || offsets[index + 1] % 3 != 0)
    {
      return false;
    }
    DrawingPrimitive primitive;
    primitive.kind = static_cast<DrawingPrimitiveKind>(kinds[index]);
    if (annotationIds[index] < 0 || annotationIds[index] >= annotations.size())
    {
      return false;
    }
    const auto item = annotations.at(annotationIds[index]).toObject();
    if (!ReportJson::text(item, "color", primitive.color, error) || !ReportJson::text(item, "text", primitive.text, error))
    {
      return false;
    }
    for (int point = offsets[index]; point < offsets[index + 1]; point += 3)
    {
      primitive.points.push_back({points[point], points[point + 1], points[point + 2]});
    }
    candidate.primitives.push_back(std::move(primitive));
  }
  output = std::move(candidate);
  return true;
}

QJsonObject labelJson(const DrawLabelData& label)
{
  QJsonArray values, anchors;
  for (const auto& value : label.values)
  {
    values.append(QJsonArray{QString::fromStdString(value.first), QString::fromStdString(value.second)});
  }
  for (const auto& anchor : label.anchors)
  {
    anchors.append(QJsonArray{anchor[0], anchor[1], anchor[2]});
  }
  return {{"step", label.stepId}, {"size", label.symbolSize}, {"type", QString::fromStdString(label.typeName)},
    {"name", QString::fromStdString(label.name)}, {"amplitude", QString::fromStdString(label.amplitude)},
    {"secondary", QString::fromStdString(label.secondaryAmplitude)}, {"values", values}, {"anchors", anchors}};
}

bool readLabel(const QJsonObject& object, DrawEntityKey key, DrawLabelData& output, std::string& error)
{
  DrawLabelData candidate;
  candidate.entity = key;
  if (!ReportJson::integer(object, "step", candidate.stepId, error) || !object.value("size").isDouble() ||
      !ReportJson::text(object, "type", candidate.typeName, error) || !ReportJson::text(object, "name", candidate.name, error) ||
      !ReportJson::text(object, "amplitude", candidate.amplitude, error) || !ReportJson::text(object, "secondary", candidate.secondaryAmplitude, error) ||
      !object.value("values").isArray() || !object.value("anchors").isArray())
  {
    return false;
  }
  candidate.symbolSize = object.value("size").toDouble();
  for (const auto value : object.value("values").toArray())
  {
    const auto pair = value.toArray();
    if (pair.size() != 2 || !pair[0].isString() || !pair[1].isString())
    {
      return false;
    }
    candidate.values.push_back({pair[0].toString().toStdString(), pair[1].toString().toStdString()});
  }
  for (const auto value : object.value("anchors").toArray())
  {
    const auto point = value.toArray();
    if (point.size() != 3 || !point[0].isDouble() || !point[1].isDouble() || !point[2].isDouble())
    {
      return false;
    }
    candidate.anchors.push_back({point[0].toDouble(), point[1].toDouble(), point[2].toDouble()});
  }
  output = std::move(candidate);
  return true;
}

bool writeScene(HDF5Tool& file, const std::string& path, const CalculiXJobSetupScene& scene)
{
  QJsonArray entities;
  int index = 0;
  for (const auto& entity : scene.symbols)
  {
    QJsonObject item = {{"feature", static_cast<int>(entity.first.feature)}, {"id", entity.first.id}};
    const auto label = scene.labels.find(entity.first);
    if (label != scene.labels.end())
    {
      item["label"] = labelJson(label->second);
    }
    entities.append(item);
    if (!writeGeometry(file, path + "/Entities/" + std::to_string(index++), entity.second))
    {
      return false;
    }
  }
  return writeJson(file, path, {{"step", scene.stepId}, {"ordinal", scene.ordinal}, {"name", QString::fromStdString(scene.name)},
    {"available", scene.available}, {"reason", QString::fromStdString(scene.unavailableReason)}, {"entities", entities}});
}

bool readScene(HDF5Tool& file, const std::string& path, CalculiXJobSetupScene& scene, hsize_t& budget, std::string& error)
{
  QJsonObject object;
  if (!readJson(file, path, object) || !ReportJson::integer(object, "step", scene.stepId, error) ||
      !ReportJson::integer(object, "ordinal", scene.ordinal, error) || !ReportJson::text(object, "name", scene.name, error) ||
      !ReportJson::text(object, "reason", scene.unavailableReason, error) || !object.value("available").isBool() || !object.value("entities").isArray())
  {
    return false;
  }
  scene.available = object.value("available").toBool();
  const auto entities = object.value("entities").toArray();
  if (entities.size() > 100000)
  {
    return false;
  }
  for (int index = 0; index < entities.size(); ++index)
  {
    const auto item = entities[index].toObject();
    int feature = -1, id = -1;
    if (!ReportJson::integer(item, "feature", feature, error) || feature < 0 || feature > 11 ||
        !ReportJson::integer(item, "id", id, error) || id <= 0)
    {
      return false;
    }
    const DrawEntityKey key = {static_cast<DrawFeature>(feature), id};
    DrawingGeometry geometry;
    if (!readGeometry(file, path + "/Entities/" + std::to_string(index), geometry, budget, error) || !scene.symbols.emplace(key, std::move(geometry)).second)
    {
      return false;
    }
    if (item.contains("label"))
    {
      DrawLabelData label;
      if (!readLabel(item.value("label").toObject(), key, label, error))
      {
        return false;
      }
      scene.labels.emplace(key, std::move(label));
    }
  }
  return true;
}

bool writeRecord(HDF5Tool& file, const std::string& path, const CalculiXJobRunRecord& record)
{
  QJsonArray files;
  int index = 0;
  for (const auto& name : record.input.order)
  {
    QJsonObject edges;
    const auto parent = record.input.includes.find(name);
    if (parent != record.input.includes.end())
    {
      for (const auto& edge : parent->second)
      {
        edges[QString::fromStdString(edge.first)] = QString::fromStdString(edge.second);
      }
    }
    files.append(QJsonObject{{"name", QString::fromStdString(name)}, {"hash", QString::fromStdString(record.input.fingerprints.at(name))}, {"includes", edges}});
    if (!writeBytes(file, path + "/Input/" + std::to_string(index++), "bytes", record.input.files.at(name)))
    {
      return false;
    }
  }
  QJsonObject metadata = {{"job", record.jobId}, {"launch", QString::fromStdString(record.launchId)},
    {"time", QString::fromStdString(record.launchedAt)}, {"directory", QString::fromStdString(record.workingDirectory)},
    {"root", QString::fromStdString(record.input.rootFile)}, {"manifest", QString::fromStdString(record.input.manifestFingerprint)},
    {"files", files}, {"reason", QString::fromStdString(record.setupUnavailableReason)}, {"setup", static_cast<bool>(record.setup)}};
  if (record.setup)
  {
    const auto& setup = *record.setup;
    if (!writeGeometry(file, path + "/Setup/Model", setup.model) || !writeScene(file, path + "/Setup/Definitions", setup.modelDefinitions))
    {
      return false;
    }
    for (std::size_t step = 0; step < setup.steps.size(); ++step)
    {
      if (!writeScene(file, path + "/Setup/Steps/" + std::to_string(step), setup.steps[step]))
      {
        return false;
      }
    }
    metadata["snapshot"] = QJsonObject{{"id", QString::fromStdString(setup.id)}, {"hash", QString::fromStdString(setup.fingerprint)},
      {"size", setup.symbolSize}, {"steps", static_cast<int>(setup.steps.size())}};
  }
  return writeJson(file, path, metadata);
}

bool readRecord(HDF5Tool& file, const std::string& path, CalculiXJobRunRecord& record, std::string& error)
{
  QJsonObject metadata;
  if (!readJson(file, path, metadata) || !ReportJson::integer(metadata, "job", record.jobId, error) ||
      !ReportJson::text(metadata, "launch", record.launchId, error) || !ReportJson::text(metadata, "time", record.launchedAt, error) ||
      !ReportJson::text(metadata, "directory", record.workingDirectory, error) || !ReportJson::text(metadata, "root", record.input.rootFile, error) ||
      !ReportJson::text(metadata, "manifest", record.input.manifestFingerprint, error) || !ReportJson::text(metadata, "reason", record.setupUnavailableReason, error) ||
      !metadata.value("files").isArray() || !metadata.value("setup").isBool())
  {
    return false;
  }
  const auto files = metadata.value("files").toArray();
  if (files.size() > 4096)
  {
    return false;
  }
  qint64 bytes = 0;
  for (int index = 0; index < files.size(); ++index)
  {
    const auto item = files[index].toObject();
    std::string name, hash;
    QByteArray data;
    if (!ReportJson::text(item, "name", name, error) || name.empty() || record.input.files.count(name) != 0 ||
        !ReportJson::text(item, "hash", hash, error) || !item.value("includes").isObject() ||
        !readBytes(file, path + "/Input/" + std::to_string(index), "bytes", 128 * 1024 * 1024 - bytes, data))
    {
      return false;
    }
    bytes += data.size();
    record.input.order.push_back(name);
    record.input.files.emplace(name, std::move(data));
    record.input.fingerprints.emplace(name, hash);
    const auto edges = item.value("includes").toObject();
    for (auto edge = edges.begin(); edge != edges.end(); ++edge)
    {
      if (!edge.value().isString())
      {
        return false;
      }
      record.input.includes[name][edge.key().toStdString()] = edge.value().toString().toStdString();
    }
  }
  if (metadata.value("setup").toBool())
  {
    auto setup = std::make_shared<CalculiXJobSetupSnapshot>();
    const auto object = metadata.value("snapshot").toObject();
    int steps = -1;
    hsize_t budget = 24000000;
    if (!ReportJson::text(object, "id", setup->id, error) || !ReportJson::text(object, "hash", setup->fingerprint, error) ||
        !object.value("size").isDouble() || !ReportJson::integer(object, "steps", steps, error) || steps < 0 || steps > 256 ||
        !readGeometry(file, path + "/Setup/Model", setup->model, budget, error) || !readScene(file, path + "/Setup/Definitions", setup->modelDefinitions, budget, error))
    {
      return false;
    }
    setup->symbolSize = object.value("size").toDouble();
    for (int step = 0; step < steps; ++step)
    {
      CalculiXJobSetupScene scene;
      if (!readScene(file, path + "/Setup/Steps/" + std::to_string(step), scene, budget, error))
      {
        return false;
      }
      setup->steps.push_back(std::move(scene));
    }
    record.setup = std::move(setup);
  }
  return CoreJobSnapshotBuilder::validate(record, error);
}
}

CoreJobRunPersistence::CoreJobRunPersistence() = default;

bool CoreJobRunPersistence::read(HDF5Tool& file, std::map<int, std::shared_ptr<const CalculiXJobRunRecord>>& output, std::string& error)
{
  try
  {
    std::map<int, std::shared_ptr<const CalculiXJobRunRecord>> candidate;
    if (!file.nameExists(Root))
    {
      if (file.nameExists(Root + "Previous"))
      {
        error = "An interrupted run-archive save needs recovery; the active snapshot group is missing.";
        return false;
      }
      output = std::move(candidate);
      error.clear();
      return true;
    }
    QJsonObject metadata;
    int version = 0;
    if (!readJson(file, Root, metadata) || !ReportJson::integer(metadata, "version", version, error) || version != 1 || !metadata.value("jobs").isArray())
    {
      error = "Invalid job run archive schema.";
      return false;
    }
    const auto jobs = metadata.value("jobs").toArray();
    if (jobs.size() > 100000)
    {
      error = "Too many archived job launches.";
      return false;
    }
    for (const auto value : jobs)
    {
      int id = 0;
      if (!ReportJson::integer({{"id", value}}, "id", id, error) || id <= 0 || candidate.count(id) != 0)
      {
        return false;
      }
      auto record = std::make_shared<CalculiXJobRunRecord>();
      if (!readRecord(file, Root + "/" + std::to_string(id), *record, error) || record->jobId != id)
      {
        if (error.empty())
        {
          error = "Invalid archived job launch payload.";
        }
        return false;
      }
      candidate.emplace(id, std::move(record));
    }
    output = std::move(candidate);
    error.clear();
    return true;
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

bool CoreJobRunPersistence::write(HDF5Tool& file, const std::map<int, std::shared_ptr<const CalculiXJobRunRecord>>& records, std::string& error)
{
  try
  {
    if (records.size() > 100000)
    {
      error = "Too many archived launches to save.";
      return false;
    }
    for (const auto& record : records)
    {
      if (!record.second || record.first != record.second->jobId || !CoreJobSnapshotBuilder::validate(*record.second, error))
      {
        return false;
      }
    }
    const auto pending = Root + "Pending";
    const auto backup = Root + "Previous";
    if (file.nameExists(pending) && !file.deleteGroup(pending))
    {
      error = "Cannot clear the staged run archive.";
      return false;
    }
    QJsonArray jobs;
    for (const auto& record : records)
    {
      jobs.append(record.first);
      if (!writeRecord(file, pending + "/" + std::to_string(record.first), *record.second))
      {
        error = "Cannot write run archive payload.";
        return false;
      }
    }
    if (!writeJson(file, pending, {{"version", 1}, {"jobs", jobs}}))
    {
      error = "Cannot write run archive manifest.";
      return false;
    }
    if (file.nameExists(backup) && !file.deleteGroup(backup))
    {
      error = "Cannot remove previous archive staging group.";
      return false;
    }
    const bool existing = file.nameExists(Root);
    if (existing && !file.renameGroup(Root, backup))
    {
      error = "Cannot stage the previous run archive.";
      return false;
    }
    if (!file.renameGroup(pending, Root))
    {
      if (existing)
      {
        file.renameGroup(backup, Root);
      }
      error = "Cannot commit the run archive.";
      return false;
    }
    if (existing && !file.deleteGroup(backup))
    {
      error = "Cannot clean the previous run archive staging group.";
      return false;
    }
    error.clear();
    return true;
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
