#ifndef GUITIMER_HPP
#define GUITIMER_HPP

#include <QTimer>

//class CalculiXCoreInterface;

class GuiTimer : public QTimer
{

public:
  GuiTimer();
  ~GuiTimer();

  //CalculiXCoreInterface *ccx_iface;
  //bool bool_init_pythoninterface = false; // will be set to true after init
  //bool bool_init_materiallibrary = false; // will be set to true after init

public slots:
  void timeoutaction();
};

#endif // GUITIMER_HPP