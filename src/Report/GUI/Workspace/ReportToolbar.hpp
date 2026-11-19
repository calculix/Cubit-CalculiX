#pragma once
#include <QToolBar>
#include <functional>
#include <string>
class Claro;
class QAction;

class ReportToolbar final : public QToolBar
{
public:
  ReportToolbar(QWidget* parent, const QIcon& pdfIcon, const QIcon& odtIcon,
    std::function<void(const std::string&)> exportReport);
  bool bind_native_actions(Claro* host, std::string& error);
  void set_export_enabled(bool enabled);

private:
  QAction* pdfAction;
  QAction* odtAction;
  bool nativeBound = false;
};
