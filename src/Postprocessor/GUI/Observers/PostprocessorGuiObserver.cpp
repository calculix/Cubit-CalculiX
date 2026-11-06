#include "PostprocessorGuiObserver.hpp"

#include <QTimer>
#include <utility>

PostprocessorGuiObserver::PostprocessorGuiObserver(std::function<void()> refresh)
  : refresh(std::move(refresh))
{
  register_observer();
}

PostprocessorGuiObserver::~PostprocessorGuiObserver()
{
  unregister_observer();
}

void PostprocessorGuiObserver::notify_command_complete()
{
  if (pending)
  {
    return;
  }

  pending = true;
  QTimer::singleShot(0, this, [this]()
  {
    pending = false;
    refresh();
  });
}
