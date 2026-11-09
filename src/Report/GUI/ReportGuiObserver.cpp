#include "ReportGuiObserver.hpp"
#include <utility>
#include <QTimer>

ReportGuiObserver::ReportGuiObserver(std::function<void()> callback) : refresh(std::move(callback))
{
  register_observer();
}

ReportGuiObserver::~ReportGuiObserver()
{
  unregister_observer();
}

void ReportGuiObserver::notify_command_complete()
{
  if (!pending)
  {
    pending = true;
    QTimer::singleShot(0, this, [this]()
    {
      pending = false;
      refresh();
    });
  }
}
