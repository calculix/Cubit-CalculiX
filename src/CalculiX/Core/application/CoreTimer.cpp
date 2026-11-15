#include "Core/application/CoreTimer.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "CubitMessage.hpp"

CoreTimer::CoreTimer()
{
     
  connect(this, &QTimer::timeout, this, &CoreTimer::timeoutaction);
  
  this->start(1000);
  
}

CoreTimer::~CoreTimer()
{}

void CoreTimer::timeoutaction()
{
  CalculiXJobsInterface ccx_jobs_iface;

  ccx_jobs_iface.check_jobs();
}
