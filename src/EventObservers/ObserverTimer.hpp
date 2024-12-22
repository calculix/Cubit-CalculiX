#ifndef OBSERVERTIMER_HPP
#define OBSERVERTIMER_HPP

#include <QTimer>

class CalculiXCoreInterface;
class CalculiXComp;

class ObserverTimer : public QTimer
{

public:
  ObserverTimer(CalculiXComp *CalculiXComp);
  ~ObserverTimer();

  CalculiXCoreInterface *ccx_iface;
  CalculiXComp *ccx_comp;

public slots:
  void timeoutaction();
};

#endif //OBSERVERTIMER_HPP