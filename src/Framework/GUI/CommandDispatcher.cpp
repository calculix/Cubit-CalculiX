#include "CommandDispatcher.hpp"
#include "Claro.hpp"

#include <QMetaObject>
#include <QPlainTextEdit>
#include <QThread>

bool CommandDispatcher::submit(Claro* host, const QString& command, std::string& error)
{
  if (host == nullptr || command.trimmed().isEmpty() || QThread::currentThread() != host->thread())
  {
    error = "Workspace commands require the active Cubit GUI thread.";
    return false;
  }

  auto* window = host->findChild<QPlainTextEdit*>("mCommandWindow");

  if (window == nullptr || !QMetaObject::invokeMethod(window, "commandEntered", Qt::DirectConnection, Q_ARG(QString, command)))
  {
    error = "Cannot submit a command through the Cubit command widget.";
    return false;
  }

  QMetaObject::invokeMethod(window, "resetHistoryIndex", Qt::DirectConnection);
  return true;
}
