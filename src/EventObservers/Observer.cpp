/*!
 * \file   Observer.cpp
 * \author Norbert Hofbauer
 * \date   8 Nov 2022
 */

#include "Observer.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CalculiXComp.hpp"

Observer::Observer()
{
  CalculiXCoreInterface *ccx_iface =  new CalculiXCoreInterface; 
}

Observer::~Observer()
{
}

void Observer::notify_model_reset()
{
  ccx_iface->core_reset();
  ccx_comp->reset();
}
void Observer::notify_command_complete()
{
  ccx_iface->core_update();
  ccx_comp->update();
}

void Observer::fetch_comp(CalculiXComp *CalculiXComp)
{
  ccx_comp = CalculiXComp;
}