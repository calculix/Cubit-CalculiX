#include "ReportProfileCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ReportProfileCommand::ReportProfileCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportProfileCommand::~ReportProfileCommand() = default;

std::vector<std::string> ReportProfileCommand::get_syntax()
{
  return {"report profile {save|load|default} <string:type='quoted',number='1',label='profile_name'>",
    "report profile list", "report profile reset"};
}

std::vector<std::string> ReportProfileCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportProfileCommand::get_help()
{
  return {"Save, load or select a workspace profile stored in Framework HDF5. Loading changes configuration and requires update."};
}

bool ReportProfileCommand::execute(CubitCommandData& data)
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

bool ReportProfileCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  std::string name;
  const std::string action = data.find_keyword("LIST") ? "list" : data.find_keyword("RESET") ? "reset" : data.find_keyword("SAVE") ? "save" : data.find_keyword("LOAD") ? "load" : "default";
  if (action != "list" && action != "reset" && !data.get_string("profile_name", name))
  {
    error = "A profile operation needs a quoted profile name.";
    return false;
  }
  std::vector<ReportProfile> profiles;
  if (!active->profile(action, name, profiles, error))
  {
    return false;
  }
  for (const auto& profile : profiles)
  {
    PRINT_INFO("Report profile %s (revision %d)\n", profile.name.c_str(), profile.revision);
  }
  return true;
}
