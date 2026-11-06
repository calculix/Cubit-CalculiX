#ifndef CUBITSIM_STARTUPTRACE_HPP
#define CUBITSIM_STARTUPTRACE_HPP

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>

inline void startup_trace(const QString& message)
{
  const QString path = qEnvironmentVariable("CUBITSIM_STARTUP_TRACE");

  if (path.isEmpty())
  {
    return;
  }

  QFile file(path);

  if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
  {
    file.write((QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + " PID=" + QString::number(QCoreApplication::applicationPid()) + " " + message + "\n").toUtf8());
    file.flush();
  }
}

#endif // CUBITSIM_STARTUPTRACE_HPP
