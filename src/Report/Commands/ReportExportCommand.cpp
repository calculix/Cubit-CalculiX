#include "ReportExportCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ReportExportCommand::ReportExportCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportExportCommand::~ReportExportCommand() = default;

std::vector<std::string> ReportExportCommand::get_syntax()
{
  return {"report export {pdf|odt|both} <string:type='quoted',number='1',label='filename'>"};
}

std::vector<std::string> ReportExportCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportExportCommand::get_help()
{
  return {"Export the current generated report. For both, filename is the base name; .pdf and .odt are appended."};
}

bool ReportExportCommand::execute(CubitCommandData& data)
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

bool ReportExportCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  std::string filename;
  if (!data.get_string("filename", filename))
  {
    error = "Report export needs a quoted destination.";
    return false;
  }
  return active->export_document(data.find_keyword("PDF") ? "pdf" : data.find_keyword("ODT") ? "odt" : "both", filename, error);
}
