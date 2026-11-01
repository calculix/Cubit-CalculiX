#ifndef GUISENDCMD_HPP
#define GUISENDCMD_HPP

#include <QPlainTextEdit>
#include <QString>

class GuiSendCmd
{

public:
  GuiSendCmd();
  ~GuiSendCmd();

  static bool gui_cmd(QString cmd);
};


#endif // GUISENDCMD_HPP