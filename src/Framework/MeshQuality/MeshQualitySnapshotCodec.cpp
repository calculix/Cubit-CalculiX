#include <utility>
#include "MeshQualitySnapshotCodec.hpp"
#include <QDataStream>
#include <QCryptographicHash>
#include <QIODevice>
#include <QString>
MeshQualitySnapshotCodec::MeshQualitySnapshotCodec() = default;
bool MeshQualitySnapshotCodec::encode(const MeshQualitySnapshot& snapshot, QByteArray& output, std::string& error)
{
  if (!snapshot.validate(error))
  {
    return false;
  }
  QByteArray bytes;
  QDataStream stream(&bytes, QIODevice::WriteOnly);
  stream.setVersion(QDataStream::Qt_6_0);
  stream << quint32(1) << QString::fromStdString(snapshot.mesh_fingerprint)
    << QString::fromStdString(snapshot.input_fingerprint) << QString::fromStdString(snapshot.coordinate_unit)
    << QString::fromStdString(snapshot.evaluator_version) << quint64(snapshot.node_ids.size());
  for (std::size_t index = 0; index < snapshot.node_ids.size(); ++index)
  {
    stream << qint64(snapshot.node_ids[index]);
    for (double coordinate : snapshot.coordinates[index])
    {
      stream << coordinate;
    }
    if (bytes.size() > maximum_bytes)
    {
      error = "Quality archive exceeds 256 MiB.";
      return false;
    }
  }
  stream << quint64(snapshot.blocks.size());
  for (const auto& block : snapshot.blocks)
  {
    stream << qint32(block.block_id) << QString::fromStdString(block.block_name)
      << QString::fromStdString(block.family) << quint64(block.nodes_per_element) << quint64(block.element_ids.size());
    for (auto id : block.element_ids)
    {
      stream << qint64(id);
    }
    for (auto id : block.connectivity)
    {
      stream << qint64(id);
    }
    stream << quint64(block.values.size());
    for (const auto& metric : block.values)
    {
      stream << QString::fromStdString(metric.first);
      for (double value : metric.second)
      {
        stream << value;
      }
      if (bytes.size() > maximum_bytes)
      {
        error = "Quality archive exceeds 256 MiB.";
        return false;
      }
    }
    stream << quint64(block.unavailable.size());
    for (const auto& issue : block.unavailable)
    {
      stream << QString::fromStdString(issue.first) << QString::fromStdString(issue.second);
    }
  }
  stream << quint64(snapshot.diagnostics.size());
  for (const auto& message : snapshot.diagnostics)
  {
    stream << QString::fromStdString(message);
  }
  if (stream.status() != QDataStream::Ok || bytes.size() > maximum_bytes - 32)
  {
    error = "Quality archive encoding failed or exceeds 256 MiB.";
    return false;
  }
  bytes.append(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256));
  output = std::move(bytes);
  error.clear();
  return true;
}
bool MeshQualitySnapshotCodec::decode(const QByteArray& input,
  std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error)
{
  if (input.size() < 36 || input.size() > maximum_bytes ||
      QCryptographicHash::hash(input.first(input.size() - 32), QCryptographicHash::Sha256) != input.last(32))
  {
    error = "Invalid quality archive size or checksum.";
    return false;
  }
  QDataStream stream(input.first(input.size() - 32));
  stream.setVersion(QDataStream::Qt_6_0);
  auto candidate = std::make_shared<MeshQualitySnapshot>();
  const auto text = [&](std::string& value)
  {
    QString saved;
    stream >> saved;
    if (saved.size() > 4096 || saved.contains(QChar('\0')))
    {
      return false;
    }
    value = saved.toStdString();
    return stream.status() == QDataStream::Ok;
  };
  const auto count = [&](quint64 maximum, quint64& value)
  {
    stream >> value;
    return stream.status() == QDataStream::Ok && value <= maximum;
  };
  quint32 version = 0;
  quint64 nodes = 0, blocks = 0;
  stream >> version;
  if (version != 1 || !text(candidate->mesh_fingerprint) || !text(candidate->input_fingerprint) ||
      !text(candidate->coordinate_unit) || !text(candidate->evaluator_version) ||
      !count(8000000, nodes) || nodes > quint64(input.size() / 32))
  {
    error = "Invalid quality archive header.";
    return false;
  }
  candidate->node_ids.resize(nodes);
  candidate->coordinates.resize(nodes);
  for (quint64 index = 0; index < nodes; ++index)
  {
    qint64 id = 0;
    stream >> id;
    candidate->node_ids[index] = id;
    for (auto& coordinate : candidate->coordinates[index])
    {
      stream >> coordinate;
    }
  }
  if (!count(100000, blocks))
  {
    error = "Invalid quality block count.";
    return false;
  }
  quint64 totalElements = 0, totalValues = 0;
  for (quint64 index = 0; index < blocks; ++index)
  {
    MeshQualityBlock block;
    qint32 id = 0;
    quint64 order = 0, elements = 0, metrics = 0, issues = 0;
    stream >> id;
    block.block_id = id;
    if (!text(block.block_name) || !text(block.family) || !count(64, order) || order == 0 ||
        !count(2000000, elements) || (totalElements += elements) > 2000000 ||
        elements * (order + 1) > quint64(stream.device()->bytesAvailable() / 8))
    {
      error = "Invalid quality block header or topology size.";
      return false;
    }
    block.nodes_per_element = order;
    block.element_ids.resize(elements);
    block.connectivity.resize(elements * order);
    for (auto& element : block.element_ids)
    {
      qint64 value = 0;
      stream >> value;
      element = value;
    }
    for (auto& node : block.connectivity)
    {
      qint64 value = 0;
      stream >> value;
      node = value;
    }
    if (!count(128, metrics))
    {
      error = "Invalid quality metric count.";
      return false;
    }
    for (quint64 metric = 0; metric < metrics; ++metric)
    {
      std::string key;
      totalValues += elements;
      if (!text(key) || key.empty() || block.values.count(key) != 0 || totalValues > 32000000 ||
          elements > quint64(stream.device()->bytesAvailable() / 8))
      {
        error = "Invalid quality metric identity or sample budget.";
        return false;
      }
      auto& values = block.values[key];
      values.resize(elements);
      for (auto& value : values)
      {
        stream >> value;
      }
    }
    if (!count(128, issues))
    {
      error = "Invalid quality diagnostics count.";
      return false;
    }
    for (quint64 issue = 0; issue < issues; ++issue)
    {
      std::string key, message;
      if (!text(key) || !text(message) || !block.unavailable.emplace(key, message).second)
      {
        error = "Invalid quality diagnostic.";
        return false;
      }
    }
    candidate->blocks.push_back(std::move(block));
  }
  quint64 diagnostics = 0;
  if (!count(100000, diagnostics))
  {
    error = "Invalid quality diagnostics count.";
    return false;
  }
  for (quint64 index = 0; index < diagnostics; ++index)
  {
    std::string message;
    if (!text(message))
    {
      error = "Invalid quality diagnostic text.";
      return false;
    }
    candidate->diagnostics.push_back(std::move(message));
  }
  if (stream.status() != QDataStream::Ok || !stream.atEnd() || !candidate->validate(error))
  {
    if (error.empty())
    {
      error = "Truncated or invalid quality archive.";
    }
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
