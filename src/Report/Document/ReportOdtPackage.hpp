#pragma once
#include <QByteArray>
#include <QtGlobal>
#include <string>
class QIODevice;
class ReportProgress;

// Stored classic ZIP entries, with mimetype first. Payloads are seekable and
// read twice (CRC then copy); only central-directory metadata stays resident.
class ReportOdtPackage
{
public:
  explicit ReportOdtPackage(QIODevice& output);
  bool begin(quint64 payloadBytes, quint64 directoryBytes, std::string& error);
  bool add_file(const QByteArray& name, QIODevice& input,
    ReportProgress* progress, std::string& error);
  bool finish(ReportProgress* progress, std::string& error);
  // ZIP64 sentinel values are intentionally excluded. No narrowing precedes
  // this check. The exporter uses generated ASCII package-relative names.
  static bool supports_entry(quint64 count, quint64 bytes,
    quint64 offset, quint64 nameBytes) noexcept;

private:
  bool checkpoint(const std::string& stage, ReportProgress* progress, std::string& error);
  QIODevice& output;
  QByteArray directory;
  quint64 totalWork = 0;
  quint64 completedWork = 0;
  quint64 expectedDirectory = 0;
  quint64 entryCount = 0;
  bool started = false;
  bool finished = false;
};
