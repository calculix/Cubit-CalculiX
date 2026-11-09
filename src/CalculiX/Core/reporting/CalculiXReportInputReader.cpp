#include "CalculiXReportInputReader.hpp"
#include <utility>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonObject>
#include <QRegularExpression>
#include <functional>
#include <set>

CalculiXReportInputReader::CalculiXReportInputReader() = default;

bool CalculiXReportInputReader::read(const ReportSource& source, std::string& error)
{
  if (source.input_files.empty())
  {
    error = "CalculiX report source has no input deck.";
    return false;
  }
  QJsonArray candidate;
  ReportSource resolved = source;
  resolved.input_files.clear();
  resolved.fingerprints.clear();
  std::set<QString> stack;
  std::set<QString> recorded;
  qint64 total = 0;
  int step = 0;
  QString stepName;
  std::function<bool(QString, int)> readFile = [&](QString path, int depth)
  {
    path = QFileInfo(path).canonicalFilePath();
    QFile file(path);
    if (path.isEmpty() || depth > 32 || stack.count(path) != 0 || !file.open(QIODevice::ReadOnly))
    {
      error = "Cannot read report input or cyclic INCLUDE: " + path.toStdString();
      return false;
    }
    total += file.size();
    if (total > 128 * 1024 * 1024)
    {
      error = "Report input exceeds the 128 MiB reading limit.";
      return false;
    }
    const QByteArray bytes = file.readAll();
    if (file.error() != QFileDevice::NoError)
    {
      error = "Cannot read CalculiX input bytes.";
      return false;
    }
    if (recorded.insert(path).second)
    {
      resolved.input_files.push_back(path.toStdString());
      resolved.fingerprints.push_back(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex().toStdString());
    }
    stack.insert(path);
    QJsonObject record;
    QJsonArray data;
    auto flush = [&]()
    {
      if (!record.isEmpty())
      {
        record["data"] = data;
        candidate.append(record);
      }
      record = QJsonObject();
      data = QJsonArray();
    };
    int lineNumber = 0;
    for (QString line : QString::fromUtf8(bytes).split('\n'))
    {
      ++lineNumber;
      line = line.trimmed();
      if (line.startsWith("**** STEP", Qt::CaseInsensitive))
      {
        const auto index = line.indexOf("NAME:", 0, Qt::CaseInsensitive);
        if (index >= 0)
        {
          stepName = line.mid(index + 5).trimmed();
        }
      }
      if (line.isEmpty() || line.startsWith("**"))
      {
        continue;
      }
      if (line.startsWith('*'))
      {
        flush();
        const QString keyword = line.section(',', 0, 0).mid(1).trimmed().toUpper();
        if (keyword == "INCLUDE")
        {
          const QRegularExpression expression("(?:INPUT|FILE)\\s*=\\s*(.+)$", QRegularExpression::CaseInsensitiveOption);
          const auto match = expression.match(line);
          QString included = match.captured(1).trimmed();
          if ((included.startsWith('"') && included.endsWith('"')) || (included.startsWith('\'') && included.endsWith('\'')))
          {
            included = included.mid(1, included.size() - 2);
          }
          if (!match.hasMatch() || included.isEmpty() || included.contains('<') ||
              !readFile(QDir(QFileInfo(path).absolutePath()).absoluteFilePath(included), depth + 1))
          {
            if (error.empty())
            {
              error = "Invalid INCLUDE in " + path.toStdString();
            }
            return false;
          }
          continue;
        }
        if (keyword == "STEP")
        {
          ++step;
        }
        record = {{"keyword", keyword}, {"header", line}, {"file", path}, {"line", lineNumber},
          {"step", step}, {"step_name", stepName}};
        if (keyword == "END STEP")
        {
          flush();
          stepName.clear();
        }
      }
      else if (!record.isEmpty())
      {
        data.append(line);
      }
      else
      {
        error = "Input data without a keyword at " + path.toStdString() + ":" + std::to_string(lineNumber);
        return false;
      }
    }
    flush();
    stack.erase(path);
    return true;
  };
  if (!readFile(QString::fromStdString(source.input_files.front()), 0))
  {
    return false;
  }
  input_records = std::move(candidate);
  input_source = std::move(resolved);
  error.clear();
  return true;
}

const QJsonArray& CalculiXReportInputReader::records() const noexcept
{
  return input_records;
}

const ReportSource& CalculiXReportInputReader::resolved_source() const noexcept
{
  return input_source;
}
