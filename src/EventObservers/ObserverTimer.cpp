#include "ObserverTimer.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CalculiXComp.hpp"

ObserverTimer::ObserverTimer(CalculiXComp *CalculiXComp)
{ 
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  this->ccx_comp = CalculiXComp;    
  connect(this, &QTimer::timeout, this, &ObserverTimer::timeoutaction);
  this->start(1000);
}

ObserverTimer::~ObserverTimer()
{}

void ObserverTimer::timeoutaction()
{
  //std::string log;
  //log =  "observer timer \n";
  //PRINT_INFO("%s", log.c_str());

  if (!ccx_iface->gui_updated())
  {
    ccx_comp->update();
    ccx_iface->set_gui_updated(true);
    this->stop();
    //std::string log;
    //log =  "gui timer update \n";
    //PRINT_INFO("%s", log.c_str());
  }
}