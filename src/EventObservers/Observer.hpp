/*!
 * \file   MyObserver.hpp
 * \brief
 *   Provides an example of how to listen to events from Cubit.
 * \author Michael Plooster
 * \date   4 Aug 2014
 */

#ifndef OBSERVER_HPP
#define OBSERVER_HPP

#include "CubitInterface.hpp"

// The MyObserver class re-implements methods that we are interested in.
// These methods are called by Cubit in response to events. For example,
// when a user saves their model to a .cub file, a save event is generated
// and the method "notify_cub_file_saved" is executed.
//
// Note that the component still needs to call the register_observer and
// unregister_observer methods of the base class in order to receive (or
// stop) event notification.

class CalculiXCoreInterface;
class CalculiXComp;

class Observer : public CubitInterface::ObserverBase
{
public:
  Observer();
  ~Observer();

  CalculiXCoreInterface *ccx_iface;
  CalculiXComp *ccx_comp;

  void notify_model_reset();
  void notify_command_complete();
  void fetch_comp(CalculiXComp *CalculiXComp);

  //observe saving and loading of cub files
  void notify_cub_file_read(std::string filename);
  void notify_cub_file_saved(std::string filename, bool is_tmp_file);
};

#endif // OBSERVER_HPP
