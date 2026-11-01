#ifndef OBSERVERTIMER_HPP
#define OBSERVERTIMER_HPP

#include <QTimer>
class CalculiXGui;

class ObserverTimer : public QTimer
{

public:
  ObserverTimer(CalculiXGui *CalculiXGui);
  ~ObserverTimer();

  CalculiXGui *ccx_comp;

public slots:
  void timeoutaction();
};

#endif //OBSERVERTIMER_HPP
