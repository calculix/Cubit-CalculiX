#include "PostprocessorModelObserver.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include <exception>
#include <utility>

PostprocessorModelObserver::PostprocessorModelObserver(std::function<void()> callback)
  : reset(std::move(callback))
{
  register_observer();
}

PostprocessorModelObserver::~PostprocessorModelObserver()
{
  unregister_observer();
}

void PostprocessorModelObserver::notify_model_reset()
{
  if (resetting)
  {
    return;
  }
  QScopedValueRollback<bool> guard(resetting, true);
  try
  {
    reset();
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot reset Postprocessor: %s\n", exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot reset Postprocessor: unknown failure.\n");
  }
}
