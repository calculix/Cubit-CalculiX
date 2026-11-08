#ifndef POSTPROCESSOR_MODELOBSERVER_HPP
#define POSTPROCESSOR_MODELOBSERVER_HPP

#include "CubitInterface.hpp"
#include <functional>

// Module-owned: model lifecycle observation remains active without GUI widgets.
class PostprocessorModelObserver : public CubitInterface::ObserverBase
{
public:
  explicit PostprocessorModelObserver(std::function<void()> reset);
  ~PostprocessorModelObserver() override;
  void notify_model_reset() override;

private:
  std::function<void()> reset;
  bool resetting = false;
};

#endif
