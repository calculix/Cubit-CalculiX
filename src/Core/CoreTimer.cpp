#include "CoreTimer.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

CoreTimer::CoreTimer()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
     
  connect(this, &QTimer::timeout, this, &CoreTimer::timeoutaction);
  
  this->start(1000);
  
}

CoreTimer::~CoreTimer()
{}

void CoreTimer::timeoutaction()
{
  ccx_iface->check_jobs();
  if (!this->bool_init_pythoninterface)
  {
    this->bool_init_pythoninterface = ccx_iface->init_pythoninterface();
  }
}