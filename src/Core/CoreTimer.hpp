#ifndef CORETIMER_HPP
#define CORETIMER_HPP

#include <vector>
#include <string>
#include <QTimer>

class CalculiXCoreInterface;

class CoreTimer : public QTimer
{

public:
  CoreTimer();
  ~CoreTimer();

  CalculiXCoreInterface *ccx_iface;
  bool bool_init_pythoninterface = false; // will be set to true after init
  bool bool_init_materiallibrary = false; // will be set to true after init

public slots:
  void timeoutaction();
};

#endif // CORETIMER_HPP