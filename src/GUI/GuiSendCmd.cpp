#include "GuiSendCmd.hpp"
#include "Claro.hpp"
#include <QMetaObject>

bool GuiSendCmd::gui_cmd(QString cmd)
{
  if (cmd.trimmed().isEmpty())
    return false;

  Claro* gui = Claro::instance();
  if (!gui)
    return false;

  auto* window = gui->findChild<QPlainTextEdit*>("mCommandWindow");
  if (!window)
    return false;

  const bool submitted = QMetaObject::invokeMethod(window, "commandEntered", Qt::DirectConnection, Q_ARG(QString, cmd));

  if (submitted)
    QMetaObject::invokeMethod(window, "resetHistoryIndex",Qt::DirectConnection);

  return submitted;
}