/*!
 * \file   MyObserver.hpp
 * \brief
 *   Provides an example of how to listen to events from Cubit.
 * \author Michael Plooster
 * \date   4 Aug 2014
 */

#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include "StopWatch.hpp"

// GUI timing handler called explicitly by the plugin's core observer.
class CalculiXComp;
class ObserverTimer;

class Observer
{
public:
  Observer();
  ~Observer();

  CalculiXComp *ccx_comp = nullptr;
  ObserverTimer *timer = nullptr;
  
  StopWatch watch;
  int time_iterator = 0;
  int last_update = 0;

  void notify_command_start();
  void before_core_update();
  void on_core_updated();
  void fetch_comp(CalculiXComp *CalculiXComp);
};

#endif // OBSERVER_HPP
