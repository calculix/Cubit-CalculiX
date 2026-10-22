#ifndef OBSERVERTIMER_HPP
#define OBSERVERTIMER_HPP

#include <QTimer>
class CalculiXComp;

class ObserverTimer : public QTimer
{

public:
  ObserverTimer(CalculiXComp *CalculiXComp);
  ~ObserverTimer();

  CalculiXComp *ccx_comp;

public slots:
  void timeoutaction();
};

#endif //OBSERVERTIMER_HPP