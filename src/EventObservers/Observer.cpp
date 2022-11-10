/*!
 * \file   Observer.cpp
 * \author Norbert Hofbauer
 * \date   8 Nov 2022
 */

#include "Observer.hpp"
#include "CalculiXCoreInterface.hpp"

Observer::Observer()
{
}

Observer::~Observer()
{
}

void Observer::notify_model_reset()
{
  CalculiXCoreInterface ccx_iface;
  ccx_iface.core_reset();
}
void Observer::notify_command_complete()
{
  CalculiXCoreInterface ccx_iface;
  ccx_iface.core_update();
}

