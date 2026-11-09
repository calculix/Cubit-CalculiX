#include "ReportUpdateCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ReportUpdateCommand::ReportUpdateCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportUpdateCommand::~ReportUpdateCommand() = default;

std::vector<std::string> ReportUpdateCommand::get_syntax()
{
  return {"report update"};
}

std::vector<std::string> ReportUpdateCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportUpdateCommand::get_help()
{
  return {"Regenerate the active report from unchanged source fingerprints, preserving notes. Changed sources require explicit new generation."};
}

bool ReportUpdateCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto active = application.lock();
    if (!active)
    {
      error = "The Report application is unavailable.";
    }
    else if (run(data, active, error))
    {
      PRINT_INFO("Report operation completed.\n");
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  catch (...)
  {
    error = "Unknown Report failure.";
  }
  PRINT_ERROR("Report: %s\n", error.empty() ? "Invalid command arguments." : error.c_str());
  return false;
}

bool ReportUpdateCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  return active->update(error);
}
