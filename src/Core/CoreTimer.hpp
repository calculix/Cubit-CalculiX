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

public slots:
  void timeoutaction();
};

#endif // CORETIMER_HPP