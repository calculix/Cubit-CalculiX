#ifndef SIMRESULTS_RESULTFILEIDENTITY_HPP
#define SIMRESULTS_RESULTFILEIDENTITY_HPP
#include <QCryptographicHash>
#include <QByteArrayView>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QStringList>
#include <QXmlStreamReader>
#include <vector>
#include <string>
namespace SimResults
{
  // SHA-256 over ordered dependency content digests, including duplicate content
  // at distinct dependency paths. Paths themselves are excluded for relocation.
  // Caller supplies all INP includes; VTK XML collection dependencies expand here.
  inline bool result_file_identity(const std::vector<std::string>& files, std::string& identity, std::string& error)
  {
    QStringList pending;
    for (const auto& path : files)
    {
      pending.append(QString::fromUtf8(path.c_str()));
    }
    QSet<QString> seen;
    QCryptographicHash aggregate(QCryptographicHash::Sha256);
    aggregate.addData(QByteArrayView("CubitSim-file-bundle-v1"));
    for (qsizetype index = 0; index < pending.size(); ++index)
    {
      const QFileInfo info(pending[index]);
      const auto path = info.canonicalFilePath();
      if (path.isEmpty() || !info.isFile() || !info.isReadable())
      {
        error = "A result dependency is missing or unreadable: "+pending[index].toUtf8().toStdString();
        return false;
      }
      if (seen.contains(path))
      {
        continue;
      }
      if (seen.size() >= 100000)
      {
        error = "Result dependency count exceeds the supported limit.";
        return false;
      }
      seen.insert(path);
      QFile file(path);
      QCryptographicHash content(QCryptographicHash::Sha256);
      if (!file.open(QIODevice::ReadOnly) || !content.addData(&file))
      {
        error = "Cannot read result dependency: "+path.toUtf8().toStdString();
        return false;
      }
      aggregate.addData(content.result());
      const auto extension = info.suffix().toLower();
      if (extension == "vtpc" || extension == "vtm" || extension == "pvtu" || extension == "vtpd")
      {
        file.seek(0);
        QXmlStreamReader xml(&file);
        while (!xml.atEnd())
        {
          xml.readNext();
          if (xml.isStartElement())
          {
            auto dependency = xml.attributes().value("file").toString();
            if (dependency.isEmpty())
            {
              dependency = xml.attributes().value("Source").toString();
            }
            if (!dependency.isEmpty())
            {
              if (pending.size() >= 100000)
              {
                error = "Result dependency count exceeds the supported limit.";
                return false;
              }
              pending.append(info.dir().absoluteFilePath(dependency));
            }
          }
        }
        if (xml.hasError())
        {
          error = "Malformed VTK collection dependency XML.";
          return false;
        }
      }
    }
    if (seen.isEmpty())
    {
      error = "No result files were supplied.";
      return false;
    }
    identity = "sha256-bundle-v1:"+aggregate.result().toHex().toStdString();
    error.clear();
    return true;
  }
}
#endif
