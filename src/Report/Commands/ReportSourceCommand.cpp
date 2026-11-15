#include <type_traits>
#include "ReportSourceCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ReportSourceCommand::ReportSourceCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportSourceCommand::~ReportSourceCommand() = default;

std::vector<std::string> ReportSourceCommand::get_syntax()
{
  return {"report source <string:type='unquoted',number='1',label='provider'> job <value:label='job_id'>",
    "report source <string:type='unquoted',number='1',label='provider'> model"};
}

std::vector<std::string> ReportSourceCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportSourceCommand::get_help()
{
  return {"Choose a model/job source and open its stored report and settings, or configure it before first generation. Stored reports remain viewable when source files are unavailable."};
}

bool ReportSourceCommand::execute(CubitCommandData& data)
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
  PRINT_ERROR("Report: %s\n", [&]() -> std::common_type_t<decltype("Invalid command arguments."), decltype(error.c_str())> {
    if (error.empty()) {
      return "Invalid command arguments.";
    } else {
      return error.c_str();
    }
  }());
  return false;
}

bool ReportSourceCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  std::string provider;
  int job = -1;
  if (!data.get_string("provider", provider) || (!data.find_keyword("MODEL") && !data.get_value("job_id", job)))
  {
    error = "Source selection needs a registered provider and model or job.";
    return false;
  }
  ReportRequest request;
  request.provider = provider;
  if (data.find_keyword("MODEL")) {
    request.job_id = 0;
  } else {
    request.job_id = job;
  }
  if (data.find_keyword("MODEL")) {
    request.kind = "model";
  } else {
    request.kind = "job";
  }
  if (request.kind == "model") {
    request.model_id = "current";
  } else {
    request.model_id = "";
  }
  return active->choose_source(request, error);
}
