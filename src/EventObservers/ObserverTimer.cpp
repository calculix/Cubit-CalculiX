#include "ObserverTimer.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CalculiXComp.hpp"
#include "CoreObserver.hpp"

ObserverTimer::ObserverTimer(CalculiXComp *CalculiXComp)
{ 
  ccx_iface = new CalculiXCoreInterface();
  this->ccx_comp = CalculiXComp;    
  connect(this, &QTimer::timeout, this, &ObserverTimer::timeoutaction);
  // Start only after the core observer has completed synchronization.
}

ObserverTimer::~ObserverTimer()
{
  stop();
  delete ccx_iface;
  ccx_iface = nullptr;
}

void ObserverTimer::timeoutaction()
{
  if (ccx_iface->block_core_update() || ccx_iface->block_gui_update() ||
      CalculiXCoreInterface::core_observer().is_updating())
    return;

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
