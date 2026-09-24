#include "GuiTimer.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

GuiTimer::GuiTimer()
{ 
  ccx_iface = new CalculiXCoreInterface();
  connect(this, &QTimer::timeout, this, &GuiTimer::timeoutaction);
  this->start(1000);
}

GuiTimer::~GuiTimer()
{}

void GuiTimer::timeoutaction()
{
  /*
  if (!this->bool_init_pythoninterface)
  {
    this->bool_init_pythoninterface = ccx_iface->init_pythoninterface();
  }
  if (!this->bool_init_materiallibrary)
  {
    this->bool_init_materiallibrary = ccx_iface->init_materiallibrary();
  }
  */
}