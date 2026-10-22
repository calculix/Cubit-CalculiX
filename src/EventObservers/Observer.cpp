#include "Observer.hpp"
/*!
 * \file   Observer.cpp
 * \author Norbert Hofbauer
 * \date   8 Nov 2022
 */

#include "Core/application/CalculiXApplicationInterface.hpp"
#include "CalculiXComp.hpp"
#include "ObserverTimer.hpp"
#include "CubitMessage.hpp"

Observer::Observer()
{
}

Observer::~Observer()
{
  if (timer)
  {
    timer->stop();
    delete timer;
    timer = nullptr;
  }

}

void Observer::notify_command_start()
{
  if (timer)
  {
    timer->stop();
  } // Cancel a pending GUI refresh before the next command.
}

void Observer::before_core_update()
{
  CalculiXApplicationInterface ccx_application_iface;

  if (ccx_application_iface.init_completed())
  {
    last_update = watch.silent_tick();
    time_iterator += last_update;
  }
}

void Observer::on_core_updated()
{
  CalculiXApplicationInterface ccx_application_iface;

  if (!ccx_application_iface.init_completed() || ccx_application_iface.block_gui_update())
  {
    return;
  }

  if (last_update > 500 || time_iterator > 500)
  {
    ccx_comp->update();
    ccx_application_iface.set_gui_updated(true);
    time_iterator = 0;
    timer->stop();
  }
  else
  {
    ccx_application_iface.set_gui_updated(false);
    timer->start(1000);
  }
}

void Observer::fetch_comp(CalculiXComp *CalculiXComp)
{
  ccx_comp = CalculiXComp;
  timer = new ObserverTimer(ccx_comp);
}
