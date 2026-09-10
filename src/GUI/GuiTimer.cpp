#include "GuiTimer.hpp"
#include "CubitMessage.hpp"

GuiTimer::GuiTimer()
{ 
  connect(this, &QTimer::timeout, this, &GuiTimer::timeoutaction);
  this->start(1000);
}

GuiTimer::~GuiTimer()
{}

void GuiTimer::timeoutaction()
{
}