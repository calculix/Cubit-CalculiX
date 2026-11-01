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
  /*
  if (!this->bool_init_pythoninterface)
  {
    this->bool_init_pythoninterface = ccx_application_iface.init_pythoninterface();
  }
  if (!this->bool_init_materiallibrary)
  {
    this->bool_init_materiallibrary = ccx_application_iface.init_materiallibrary();
  }
  */
}