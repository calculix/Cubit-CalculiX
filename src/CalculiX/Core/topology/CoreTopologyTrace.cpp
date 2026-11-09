#include "CoreTopologyTrace.hpp"
#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QtGlobal>

void CoreTopologyTrace::write(const char* event)
{
  const QByteArray path = qgetenv("CUBITSIM_TOPOLOGY_TRACE_FILE");
  if (path.isEmpty())
  {
    return;
  }

  QFile file(QString::fromUtf8(path));
  if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
  {
    return;
  }

  QTextStream stream(&file);
  stream << QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)
         << " " << event << "\n";
  stream.flush();
  file.flush();
}
