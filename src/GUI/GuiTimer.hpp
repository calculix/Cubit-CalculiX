#ifndef GUITIMER_HPP
#define GUITIMER_HPP

#include <QTimer>

class GuiTimer : public QTimer
{

public:
  GuiTimer();
  ~GuiTimer();

public slots:
  void timeoutaction();
};

#endif // GUITIMER_HPP