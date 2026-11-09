#include "ReportGenerateCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ReportGenerateCommand::ReportGenerateCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportGenerateCommand::~ReportGenerateCommand() = default;

std::vector<std::string> ReportGenerateCommand::get_syntax()
{
  return {"report <string:type='unquoted',number='1',label='provider'> job <value:label='job_id'> [profile <string:type='quoted',number='1',label='profile_name'>]",
    "report <string:type='unquoted',number='1',label='provider'> model [profile <string:type='quoted',number='1',label='profile_name'>]"};
}

std::vector<std::string> ReportGenerateCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportGenerateCommand::get_help()
{
  return {"Generate a model/steps report using its workspace template and saved defaults. Refreshes the workspace preview without opening another window."};
}

bool ReportGenerateCommand::execute(CubitCommandData& data)
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

bool ReportGenerateCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  std::string provider, profile;
  int job = -1;
  if (!data.get_string("provider", provider) || (!data.find_keyword("MODEL") && !data.get_value("job_id", job)) ||
      (data.find_keyword("PROFILE") && !data.get_string("profile_name", profile)))
  {
    error = "Report generation needs a provider and job ID.";
    return false;
  }
  ReportRequest request;
  request.provider = provider;
  request.job_id = data.find_keyword("MODEL") ? 0 : job;
  request.kind = data.find_keyword("MODEL") ? "model" : "job";
  request.model_id = request.kind == "model" ? "current" : "";
  return active->generate_request(request, profile, error);
}
