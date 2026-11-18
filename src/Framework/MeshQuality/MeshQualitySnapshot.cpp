#include "MeshQualitySnapshot.hpp"
#include <QCryptographicHash>
#include <QDataStream>
#include <QBuffer>
#include <unordered_set>
#include <QIODevice>
#include <cmath>
#include <set>
MeshQualitySnapshot::MeshQualitySnapshot() = default;
std::string MeshQualitySnapshot::fingerprint() const
{
  if (node_ids.size() != coordinates.size())
  {
    return {};
  }
  QCryptographicHash hash(QCryptographicHash::Sha256);
  QByteArray bytes;
  QBuffer buffer(&bytes);
  buffer.open(QIODevice::WriteOnly);
  QDataStream stream(&buffer);
  stream.setVersion(QDataStream::Qt_6_0);
  const auto flush = [&]()
  {
    if (bytes.size() >= 65536)
    {
      hash.addData(bytes);
      bytes.clear();
      buffer.seek(0);
    }
  };
  stream << QString::fromStdString(coordinate_unit) << quint64(node_ids.size());
  for (std::size_t index = 0; index < node_ids.size(); ++index)
  {
    stream << qint64(node_ids[index]);
    for (double coordinate : coordinates[index])
    {
      stream << coordinate;
    }
    flush();
  }
  stream << quint64(blocks.size());
  for (const auto& block : blocks)
  {
    stream << qint32(block.block_id) << QString::fromStdString(block.family)
      << quint64(block.nodes_per_element) << quint64(block.element_ids.size());
    for (auto id : block.element_ids)
    {
      stream << qint64(id);
      flush();
    }
    for (auto id : block.connectivity)
    {
      stream << qint64(id);
      flush();
    }
  }
  hash.addData(bytes);
  return hash.result().toHex().toStdString();
}
bool MeshQualitySnapshot::validate(std::string& error) const
{
  if (node_ids.size() != coordinates.size() || node_ids.size() > 8000000 || blocks.size() > 100000)
  {
    error = "Invalid or oversized quality mesh.";
    return false;
  }
  if (coordinate_unit.size() > 256 || mesh_fingerprint.size() > 256 || input_fingerprint.size() > 256 ||
      evaluator_version.size() > 4096 || diagnostics.size() > 100000)
  {
    error = "Invalid quality snapshot metadata.";
    return false;
  }
  std::uint64_t estimatedBytes = 4096 + node_ids.size() * 32ULL;
  for (const auto& message : diagnostics)
  {
    if (message.size() > 4096)
    {
      error = "Quality diagnostic exceeds the text budget.";
      return false;
    }
    estimatedBytes += 8 + message.size() * 2ULL;
  }
  std::unordered_set<std::int64_t> nodes;
  nodes.reserve(node_ids.size());
  for (std::size_t index = 0; index < node_ids.size(); ++index)
  {
    if (node_ids[index] <= 0 || !nodes.insert(node_ids[index]).second)
    {
      error = "Repeated or invalid quality node ID.";
      return false;
    }
    for (double value : coordinates[index])
    {
      if (!std::isfinite(value))
      {
        error = "Nonfinite quality mesh coordinates.";
        return false;
      }
    }
  }
  std::size_t elements = 0;
  for (const auto& block : blocks)
  {
    elements += block.element_ids.size();
    if (block.block_name.size() > 4096 || block.values.size() > 128 || block.unavailable.size() > 128)
    {
      error = "Invalid quality block metadata.";
      return false;
    }
    estimatedBytes += 128 + block.block_name.size() * 2ULL + block.element_ids.size() * 8ULL + block.connectivity.size() * 8ULL;
    for (const auto& issue : block.unavailable)
    {
      if (issue.first.empty() || issue.first.size() > 256 || issue.second.size() > 4096)
      {
        error = "Invalid quality metric diagnostic.";
        return false;
      }
      estimatedBytes += 16 + (issue.first.size() + issue.second.size()) * 2ULL;
    }
    if (block.block_id <= 0 || block.nodes_per_element == 0 || block.nodes_per_element > 64 ||
        elements > 2000000 || block.connectivity.size() != block.element_ids.size() * block.nodes_per_element ||
        (block.family != "hex" && block.family != "tet" && block.family != "quad" && block.family != "tri"))
    {
      error = "Invalid quality block topology or resource limit.";
      return false;
    }
    std::unordered_set<std::int64_t> ids;
    ids.reserve(block.element_ids.size());
    for (auto id : block.element_ids)
    {
      if (id <= 0 || !ids.insert(id).second)
      {
        error = "Repeated or invalid quality element ID.";
        return false;
      }
    }
    for (auto id : block.connectivity)
    {
      if (nodes.count(id) == 0)
      {
        error = "Quality connectivity references a missing node.";
        return false;
      }
    }
    for (const auto& values : block.values)
    {
      estimatedBytes += 16 + values.first.size() * 2ULL + values.second.size() * 8ULL;
      if (values.first.empty() || values.first.size() > 256 || values.second.size() != block.element_ids.size())
      {
        error = "Quality values do not match element identities.";
        return false;
      }
    }
  }
  if (estimatedBytes > 256ULL * 1024 * 1024 - 32)
  {
    error = "Quality snapshot exceeds its serialized size budget.";
    return false;
  }
  if (mesh_fingerprint.empty() || fingerprint() != mesh_fingerprint)
  {
    error = "Quality mesh fingerprint mismatch.";
    return false;
  }
  error.clear();
  return true;
}
