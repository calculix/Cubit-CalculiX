#include "ReportPreviewCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ReportPreviewCommand::ReportPreviewCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportPreviewCommand::~ReportPreviewCommand() = default;

std::vector<std::string> ReportPreviewCommand::get_syntax()
{
  return {"report preview [page <value:label='page'>] [zoom <value:label='zoom'>] [fit {width|page}]"};
}

std::vector<std::string> ReportPreviewCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportPreviewCommand::get_help()
{
  return {"Open the paginated report preview window."};
}

bool ReportPreviewCommand::execute(CubitCommandData& data)
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

bool ReportPreviewCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  int page = -1, zoom = -1;
  // PAGE also occurs in FIT PAGE. Read the labeled value when present, but a
  // fit-only command needs no page number. Combined page/fit commands retain it.
  const bool fitting = data.find_keyword("FIT");
  const bool hasPage = data.find_keyword("PAGE") && data.get_value("page", page);
  if (!hasPage)
  {
    page = -1;
  }
  if ((!fitting && data.find_keyword("PAGE") && !hasPage) ||
      (data.find_keyword("ZOOM") && !data.get_value("zoom", zoom)))
  {
    error = "Preview page and zoom require integer values.";
    return false;
  }
  const std::string fit = data.find_keyword("FIT") ? (data.find_keyword("WIDTH") ? "width" : "page") : "";
  return active->request_preview(error, page, zoom, fit);
}
