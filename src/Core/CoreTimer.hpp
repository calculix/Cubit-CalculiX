#ifndef CORETIMER_HPP
#define CORETIMER_HPP

#include <QTimer>

class CalculiXCoreInterface;

class CoreTimer : public QTimer
{

public:
  CoreTimer();
  ~CoreTimer();

  CalculiXCoreInterface *ccx_iface;

public slots:
  void timeoutaction();
};

#endif // CORETIMER_HPP