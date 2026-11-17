#include "ReportOdtPackage.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include <QDataStream>
#include <QIODevice>
#include <algorithm>
#include <array>
#include <limits>

namespace
{
constexpr quint64 zipLimit = 0xffffffffull;
constexpr quint64 entryLimit = 0xffffull;

const std::array<quint32, 256>& crc_table()
{
  static const auto table = []()
  {
    std::array<quint32, 256> result{};
    for (quint32 index = 0; index < result.size(); ++index)
    {
      quint32 value = index;
      for (int bit = 0; bit < 8; ++bit)
      {
        value = (value >> 1) ^ (0xedb88320u & (0u - (value & 1u)));
      }
      result[index] = value;
    }
    return result;
  }();
  return table;
}
}

ReportOdtPackage::ReportOdtPackage(QIODevice& device) : output(device)
{
}

bool ReportOdtPackage::supports_entry(quint64 count, quint64 bytes,
  quint64 offset, quint64 nameBytes) noexcept
{
  return count > 0 && count < entryLimit && bytes < zipLimit && offset < zipLimit &&
    nameBytes > 0 && nameBytes <= entryLimit &&
    bytes < zipLimit - offset && 30 + nameBytes < zipLimit - offset - bytes;
}

bool ReportOdtPackage::begin(quint64 payloadBytes, quint64 directoryBytes, std::string& error)
{
  if (started || !output.isWritable() || output.pos() != 0 || payloadBytes >= zipLimit ||
      directoryBytes >= zipLimit ||
      directoryBytes > (std::numeric_limits<std::size_t>::max)() - 22 ||
      payloadBytes > ((std::numeric_limits<std::size_t>::max)() - directoryBytes - 22) / 2)
  {
    error = "ODT package exceeds classic ZIP limits or needs a new writable output.";
    return false;
  }
  totalWork = payloadBytes * 2 + directoryBytes + 22;
  expectedDirectory = directoryBytes;
  started = true;
  error.clear();
  return true;
}

bool ReportOdtPackage::checkpoint(const std::string& stage, ReportProgress* progress, std::string& error)
{
  return ReportProgress::advance(progress, ReportProgressStage::ExportPackage, stage,
    static_cast<std::size_t>(completedWork), static_cast<std::size_t>(totalWork), error);
}

bool ReportOdtPackage::add_file(const QByteArray& name, QIODevice& input,
  ReportProgress* progress, std::string& error)
{
  const qint64 length = input.size();
  const qint64 offset = output.pos();
  const bool first = entryCount == 0;
  if (!started || finished || length < 0 || offset < 0 || !input.isReadable() ||
      input.isSequential() || !input.seek(0) || (first && name != "mimetype") ||
      (!first && name == "mimetype") ||
      !supports_entry(entryCount + 1, static_cast<quint64>(length), static_cast<quint64>(offset),
        static_cast<quint64>(name.size())))
  {
    error = "Cannot add ODT entry within classic ZIP limits: " + name.toStdString();
    return false;
  }
  const auto chunk = static_cast<qint64>(ReportUserOptions::export_chunk_bytes());
  quint32 crc = 0xffffffffu;
  qint64 remaining = length;
  while (remaining > 0)
  {
    if (!checkpoint("Checking ODT entry " + name.toStdString(), progress, error))
    {
      return false;
    }
    const auto bytes = input.read((std::min)(remaining, chunk));
    if (bytes.isEmpty())
    {
      error = "Cannot read ODT entry: " + name.toStdString();
      return false;
    }
    for (unsigned char byte : bytes)
    {
      crc = crc_table()[(crc ^ byte) & 0xffu] ^ (crc >> 8);
    }
    remaining -= bytes.size();
    completedWork += static_cast<quint64>(bytes.size());
  }
  crc ^= 0xffffffffu;
  if (!input.seek(0))
  {
    error = "Cannot rewind ODT entry: " + name.toStdString();
    return false;
  }
  QDataStream stream(&output);
  stream.setByteOrder(QDataStream::LittleEndian);
  const auto size = static_cast<quint32>(length);
  const auto nameSize = static_cast<quint16>(name.size());
  stream << quint32(0x04034b50) << quint16(20) << quint16(0) << quint16(0) << quint16(0) << quint16(0)
    << crc << size << size << nameSize << quint16(0);
  stream.writeRawData(name.constData(), name.size());
  if (stream.status() != QDataStream::Ok)
  {
    error = "Cannot write ODT entry header: " + name.toStdString();
    return false;
  }
  remaining = length;
  while (remaining > 0)
  {
    if (!checkpoint("Writing ODT entry " + name.toStdString(), progress, error))
    {
      return false;
    }
    const auto bytes = input.read((std::min)(remaining, chunk));
    if (bytes.isEmpty() || output.write(bytes) != bytes.size())
    {
      error = "Cannot copy ODT entry: " + name.toStdString();
      return false;
    }
    remaining -= bytes.size();
    completedWork += static_cast<quint64>(bytes.size());
  }
  QDataStream central(&directory, QIODevice::WriteOnly | QIODevice::Append);
  central.setByteOrder(QDataStream::LittleEndian);
  central << quint32(0x02014b50) << quint16(20) << quint16(20) << quint16(0) << quint16(0)
    << quint16(0) << quint16(0) << crc << size << size << nameSize << quint16(0) << quint16(0)
    << quint16(0) << quint16(0) << quint32(0) << static_cast<quint32>(offset);
  central.writeRawData(name.constData(), name.size());
  if (central.status() != QDataStream::Ok || static_cast<quint64>(directory.size()) > expectedDirectory)
  {
    error = "Cannot construct the ODT package directory.";
    return false;
  }
  ++entryCount;
  return checkpoint("ODT entry complete", progress, error);
}

bool ReportOdtPackage::finish(ReportProgress* progress, std::string& error)
{
  const qint64 offset = output.pos();
  const auto bytes = static_cast<quint64>(directory.size());
  if (!started || finished || entryCount == 0 || offset < 0 || bytes != expectedDirectory)
  {
    error = "ODT package directory is incomplete or inconsistent.";
    return false;
  }
  if (static_cast<quint64>(offset) >= zipLimit || bytes >= zipLimit - static_cast<quint64>(offset) ||
      22 >= zipLimit - static_cast<quint64>(offset) - bytes)
  {
    error = "ODT package directory requires unsupported ZIP64 output.";
    return false;
  }
  qint64 written = 0;
  while (written < directory.size())
  {
    if (!checkpoint("Writing ODT directory", progress, error))
    {
      return false;
    }
    const auto count = (std::min)(static_cast<qint64>(directory.size()) - written,
      static_cast<qint64>(ReportUserOptions::export_chunk_bytes()));
    if (output.write(directory.constData() + written, count) != count)
    {
      error = "Cannot write the ODT package directory.";
      return false;
    }
    written += count;
    completedWork += static_cast<quint64>(count);
  }
  QDataStream stream(&output);
  stream.setByteOrder(QDataStream::LittleEndian);
  stream << quint32(0x06054b50) << quint16(0) << quint16(0) << static_cast<quint16>(entryCount)
    << static_cast<quint16>(entryCount) << static_cast<quint32>(bytes)
    << static_cast<quint32>(offset) << quint16(0);
  if (stream.status() != QDataStream::Ok)
  {
    error = "Cannot finish the ODT package.";
    return false;
  }
  completedWork += 22;
  if (completedWork != totalWork)
  {
    error = "ODT payload sizes changed during packaging.";
    return false;
  }
  finished = true;
  return checkpoint("ODT package complete", progress, error);
}
