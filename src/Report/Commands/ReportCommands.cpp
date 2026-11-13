#include "ReportCommands.hpp"
#include "ReportImageCommand.hpp"
#include <utility>
#include "ReportGenerateCommand.hpp"
#include "ReportSourceCommand.hpp"
#include "ReportConfigureCommand.hpp"
#include "ReportUpdateCommand.hpp"
#include "ReportPreviewCommand.hpp"
#include "ReportExportCommand.hpp"
#include "ReportProfileCommand.hpp"

ReportCommands::ReportCommands(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportCommands::~ReportCommands() = default;

std::vector<std::string> ReportCommands::command_keys() const
{
  return {"ReportImageCommand", "ReportSourceCommand", "ReportGenerateCommand", "ReportConfigureCommand", "ReportUpdateCommand", "ReportPreviewCommand", "ReportExportCommand", "ReportProfileCommand"};
}

CubitCommand* ReportCommands::create_command(const std::string& key)
{
  if (key == "ReportImageCommand")
  {
    return new ReportImageCommand(application);
  }
  if (key == "ReportSourceCommand")
  {
    return new ReportSourceCommand(application);
  }
  if (key == "ReportGenerateCommand")
  {
    return new ReportGenerateCommand(application);
  }
  if (key == "ReportConfigureCommand")
  {
    return new ReportConfigureCommand(application);
  }
  if (key == "ReportUpdateCommand")
  {
    return new ReportUpdateCommand(application);
  }
  if (key == "ReportPreviewCommand")
  {
    return new ReportPreviewCommand(application);
  }
  if (key == "ReportExportCommand")
  {
    return new ReportExportCommand(application);
  }
  if (key == "ReportProfileCommand")
  {
    return new ReportProfileCommand(application);
  }
  return nullptr;
}
