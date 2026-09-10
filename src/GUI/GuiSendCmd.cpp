#include "GuiSendCmd.hpp"
#include "CubitMessage.hpp"
#include "CubitInterface.hpp"
#include "Claro.hpp"

GuiSendCmd::GuiSendCmd()
{}

GuiSendCmd::~GuiSendCmd()
{}

bool GuiSendCmd::gui_cmd(QString cmd)
{    
  CubitInterface::cmd(cmd.toStdString().c_str());

  Claro* gui;
  gui = Claro::instance();
  QList<QPlainTextEdit *> widgets = gui->findChildren<QPlainTextEdit *>();
  foreach(QPlainTextEdit *w, widgets) {
    if (w->objectName()=="mHistoryWindow")
    {
      w->appendPlainText(cmd);
    }
  }

  return true;
}