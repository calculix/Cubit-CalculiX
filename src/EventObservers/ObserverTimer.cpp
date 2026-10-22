#include "ObserverTimer.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXComp.hpp"
#include "Core/application/CoreObserver.hpp"

ObserverTimer::ObserverTimer(CalculiXComp *CalculiXComp)
{ 
  this->ccx_comp = CalculiXComp;    
  connect(this, &QTimer::timeout, this, &ObserverTimer::timeoutaction);
  // Start only after the core observer has completed synchronization.
}

ObserverTimer::~ObserverTimer()
{
  stop();
}

void ObserverTimer::timeoutaction()
{
  CalculiXApplicationInterface ccx_application_iface;

  if (ccx_application_iface.block_core_update() || ccx_application_iface.block_gui_update() ||
      ccx_application_iface.core_observer().is_updating())
  {
    return;
  }

  //std::string log;
  //log =  "observer timer \n";
  //PRINT_INFO("%s", log.c_str());

  if (!ccx_application_iface.gui_updated())
  {
    ccx_comp->update();
    ccx_application_iface.set_gui_updated(true);
    this->stop();
    //std::string log;
    //log =  "gui timer update \n";
    //PRINT_INFO("%s", log.c_str());
  }
}
