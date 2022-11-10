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
class Observer : public CubitInterface::ObserverBase
{
public:
  Observer();
  ~Observer();

  void notify_model_reset();
  void notify_command_complete();
};

#endif // OBSERVER_HPP
