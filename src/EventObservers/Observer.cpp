/*!
 * \file   Observer.cpp
 * \author Norbert Hofbauer
 * \date   8 Nov 2022
 */

#include "Observer.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CalculiXComp.hpp"
#include "ObserverTimer.hpp"
#include "CubitMessage.hpp"

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

void Observer::notify_command_start()
{
  if (ccx_iface->init_completed())
  { 
    timer->stop(); // stop timer if next command is already executed, like running a journal
  }
}

void Observer::notify_command_complete()
{
  if (ccx_iface->init_completed())
  { 
    int last_update = this->watch.silent_tick();
    time_iterator = time_iterator + last_update;
    
    if (!ccx_iface->block_core_update())
    {
      ccx_iface->core_update();
    }
    
    /*
    std::string log;
    log =  "observer last update " + std::to_string(last_update) + " \n";
    PRINT_INFO("%s", log.c_str());
    log =  "observer time_iterator " + std::to_string(time_iterator) + " \n";
    PRINT_INFO("%s", log.c_str());
    */
    if (!ccx_iface->block_gui_update())
    {
      if ((last_update>500)||(time_iterator>500))
      {
        ccx_comp->update();
        ccx_iface->set_gui_updated(true);
        //log =  "true \n";
        //PRINT_INFO("%s", log.c_str());
        time_iterator = 0;
        timer->stop();
      }else{
        ccx_iface->set_gui_updated(false);
        //log =  "false \n";
        //PRINT_INFO("%s", log.c_str());
        timer->start(1000);
      }
    }
  }
}

void Observer::fetch_comp(CalculiXComp *CalculiXComp)
{
  ccx_comp = CalculiXComp;
  timer = new ObserverTimer(ccx_comp);
}

void Observer::notify_cub_file_read(std::string filename) 
{
  ccx_iface->core_read_cub(filename);
}
// Notification when a cub file is saved. The flag, is_tmp_file, is true when
// the file is a temporary file being used by Cubit for some internal operation
// (for example, undo).
void Observer::notify_cub_file_saved(std::string filename, bool is_tmp_file) 
{ 
  if (!is_tmp_file)
  {
    ccx_iface->core_save_cub(filename);
  }
}