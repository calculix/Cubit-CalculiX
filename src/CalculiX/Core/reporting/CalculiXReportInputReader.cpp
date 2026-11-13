#include "CalculiXReportInputReader.hpp"
#include "CalculiX/Core/jobs/CalculiXJobInputBundle.hpp"
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
  return read_input(source, nullptr, error);
}

bool CalculiXReportInputReader::read_archive(const ReportSource& source, const CalculiXJobInputBundle& archive, std::string& error)
{
  if (source.input_files.empty() || source.input_files.front() != archive.rootFile)
  {
    error = "Report source does not match the archived root input.";
    return false;
  }
  return read_input(source, &archive, error);
}

bool CalculiXReportInputReader::read_input(const ReportSource& source, const CalculiXJobInputBundle* archive, std::string& error)
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
    if (archive == nullptr)
    {
      path = QFileInfo(path).canonicalFilePath();
    }
    if (path.isEmpty() || depth > 32 || stack.count(path) != 0)
    {
      error = "Cannot read report input or cyclic INCLUDE: " + path.toStdString();
      return false;
    }
    QByteArray bytes;
    if (archive != nullptr)
    {
      const auto found = archive->files.find(path.toStdString());
      const auto fingerprint = archive->fingerprints.find(path.toStdString());
      if (found == archive->files.end() || fingerprint == archive->fingerprints.end() ||
          QCryptographicHash::hash(found->second, QCryptographicHash::Sha256).toHex().toStdString() != fingerprint->second)
      {
        error = "Missing or corrupt archived input file.";
        return false;
      }
      bytes = found->second;
    }
    else
    {
      QFile file(path);
      if (!file.open(QIODevice::ReadOnly) || file.size() > 128 * 1024 * 1024 - total)
      {
        error = "Cannot read report input or input exceeds the reading limit.";
        return false;
      }
      bytes = file.readAll();
      if (file.error() != QFileDevice::NoError)
      {
        error = "Cannot read CalculiX input bytes.";
        return false;
      }
    }
    total += bytes.size();
    if (total > 128 * 1024 * 1024)
    {
      error = "Report input exceeds the 128 MiB reading limit.";
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
          QString target;
          if (archive != nullptr)
          {
            const auto parent = archive->includes.find(path.toStdString());
            if (parent != archive->includes.end())
            {
              const auto child = parent->second.find(included.toStdString());
              if (child != parent->second.end())
              {
                target = QString::fromStdString(child->second);
              }
            }
          }
          else
          {
            target = QDir(QFileInfo(path).absolutePath()).absoluteFilePath(included);
          }
          if (!match.hasMatch() || included.isEmpty() || included.contains('<') || target.isEmpty() || !readFile(target, depth + 1))
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
