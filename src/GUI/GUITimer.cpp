#include "GUITimer.hpp"
#include "CubitMessage.hpp"

GUITimer::GUITimer()
{     
  connect(this, &QTimer::timeout, this, &GUITimer::timeoutaction);
  this->start(1000);
}

GUITimer::~GUITimer()
{}

void GUITimer::timeoutaction()
{

}