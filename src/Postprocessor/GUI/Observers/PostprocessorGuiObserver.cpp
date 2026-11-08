#include "PostprocessorGuiObserver.hpp"

#include <QTimer>
#include <utility>

PostprocessorGuiObserver::PostprocessorGuiObserver(std::function<void()> refresh, std::function<bool()> suppress)
  : refresh(std::move(refresh)), suppress(std::move(suppress))
{
  register_observer();
}

PostprocessorGuiObserver::~PostprocessorGuiObserver()
{
  unregister_observer();
}

void PostprocessorGuiObserver::notify_command_complete()
{
  // Our camera dispatcher already schedules its own viewport update. Do not
  // enqueue a second workspace pass for its synchronous command completion.
  // Normal external/source/options commands keep the authoritative refresh path.
  if ((suppress && suppress()) || pending)
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
