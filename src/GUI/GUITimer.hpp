#ifndef GUITIMER_HPP
#define GUITIMER_HPP

#include <QTimer>

class CalculiXCoreInterface;

class GUITimer : public QTimer
{

public:
  GUITimer();
  ~GUITimer();

public slots:
  void timeoutaction();
};

#endif // GUITIMER_HPP