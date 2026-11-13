#include "ReportImageCommand.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <exception>
#include <utility>
ReportImageCommand::ReportImageCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> ReportImageCommand::get_syntax()
{
  const std::string common = " <string:type='quoted',number='1',label='visual_name'>"
    " [caption <string:type='quoted',number='1',label='figure_caption'>]"
    " [width <value:label='figure_width'> height <value:label='figure_height'>]"
    " [background <string:type='unquoted',number='1',label='figure_background'>]"
    " [legend <string:type='unquoted',number='1',label='figure_legend'>]";
  return {"report image add postprocessor view" + common +
      " [labels <string:type='unquoted',number='1',label='figure_labels'>] [triad <string:type='unquoted',number='1',label='figure_triad'>]",
    "report image add postprocessor plot" + common +
      " [marker <string:type='unquoted',number='1',label='figure_marker'>]"};
}
std::vector<std::string> ReportImageCommand::get_syntax_help()
{
  return {"report image add postprocessor view|plot \"name\" [caption \"text\"] [width 1920 height 1080] [background current|white|transparent] [legend on|off] [labels on|off] [triad on|off] [marker on|off]"};
}
std::vector<std::string> ReportImageCommand::get_help()
{
  return {"Append an immutable PNG snapshot to the active generated Report's Figures section. Does not switch workspaces or generate a report."};
}
bool ReportImageCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto active = application.lock();
    ReportImageRequest request;
    request.provider = "postprocessor";
    ReportVisualReference visual;
    visual.kind = data.find_keyword("PLOT") ? ReportVisualKind::Plot : ReportVisualKind::View;
    std::string caption;
    bool valid = active && data.get_string("visual_name",visual.reference) && !visual.reference.empty();
    request.visual = visual;
    if (data.find_keyword("CAPTION"))
    {
      valid = valid && data.get_string("figure_caption",caption);
    }
    if (data.find_keyword("WIDTH") || data.find_keyword("HEIGHT"))
    {
      double width = 0, height = 0;
      valid = valid && data.get_value("figure_width",width) && data.get_value("figure_height",height) &&
        std::isfinite(width) && std::isfinite(height) && std::floor(width) == width && std::floor(height) == height &&
        width >= 64 && width <= 8192 && height >= 64 && height <= 8192;
      if (valid)
      {
        request.width = static_cast<int>(width);
        request.height = static_cast<int>(height);
      }
    }
    for (const auto* key : {"background","legend","labels","triad","marker"})
    {
      std::string keyword(key), value;
      std::transform(keyword.begin(),keyword.end(),keyword.begin(),[](unsigned char c)
      {
        return static_cast<char>(std::toupper(c));
      });
      if (data.find_keyword(keyword.c_str()))
      {
        const auto label = "figure_"+std::string(key);
        valid = data.get_string(label.c_str(),value) && valid;
        std::transform(value.begin(),value.end(),value.begin(),[](unsigned char c)
        {
          return static_cast<char>(std::tolower(c));
        });
        request.settings.values["capture."+std::string(key)] = value;
      }
    }
    if (valid && active->add_image(request,caption,error))
    {
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  PRINT_ERROR("Cannot add Report figure: %s\n",error.empty() ? "Invalid image arguments." : error.c_str());
  return false;
}
