#include "ReportModelObserver.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include <exception>

ReportModelObserver::ReportModelObserver(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
  register_observer();
}

ReportModelObserver::~ReportModelObserver()
{
  unregister_observer();
}

void ReportModelObserver::notify_model_reset()
{
  if (const auto active = application.lock())
  {
    active->reset();
  }
}

void ReportModelObserver::notify_cub_file_read(std::string filename)
{
  if (updating)
  {
    return;
  }
  QScopedValueRollback<bool> guard(updating, true);
  if (const auto active = application.lock())
  {
    std::string error;
    if (!active->read_project(filename, error))
    {
      PRINT_ERROR("Cannot restore Report workspace data: %s\n", error.c_str());
    }
  }
}

void ReportModelObserver::notify_cub_file_saved(std::string filename, bool temporary)
{
  // Follow the existing SDK integration: temporary native saves are not amended.
  // Explicit project saves embed report data. Autosave integration needs separate
  // host lifecycle verification before claiming report recovery from temp files.
  if (temporary || updating)
  {
    return;
  }
  QScopedValueRollback<bool> guard(updating, true);
  if (const auto active = application.lock())
  {
    std::string error;
    if (!active->write_project(filename, error))
    {
      PRINT_ERROR("Cubit saved the project, but Report data was not fully saved: %s\n", error.c_str());
    }
  }
}
