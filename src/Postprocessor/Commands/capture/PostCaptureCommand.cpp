#include "PostCaptureCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <exception>
#include <utility>
PostCaptureCommand::PostCaptureCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostCaptureCommand::get_syntax()
{
  const std::string options = " [width <value:label='capture_width'> height <value:label='capture_height'>]"
    " [background <string:type='unquoted',number='1',label='capture_background'>]"
    " [legend <string:type='unquoted',number='1',label='capture_legend'>]";
  return {"post capture view [<string:type='quoted',number='1',label='capture_name'>] file <string:type='quoted',number='1',label='capture_file'>" + options +
    " [labels <string:type='unquoted',number='1',label='capture_labels'>] [triad <string:type='unquoted',number='1',label='capture_triad'>]",
    "post capture plot <string:type='quoted',number='1',label='capture_name'> file <string:type='quoted',number='1',label='capture_file'>" + options +
    " [marker <string:type='unquoted',number='1',label='capture_marker'>]"};
}
std::vector<std::string> PostCaptureCommand::get_syntax_help()
{
  return {"post capture view [\"view1\"] file \"new.png\" [width 1920 height 1080] [background current|white|transparent] [legend on|off] [labels on|off] [triad on|off]",
    "post capture plot \"History\" file \"new.png\" [width 1920 height 1080] [background current|white|transparent] [legend on|off] [marker on|off]"};
}
std::vector<std::string> PostCaptureCommand::get_help()
{
  return {"Export a frozen viewport or plot to a new PNG without changing the live workspace. Dimensions: 64..8192 per side, at most 32 million pixels. Existing files are never overwritten."};
}
bool PostCaptureCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto active = application.lock();
    Postprocessor::CaptureDefinition request;
    request.target = data.find_keyword("PLOT") ? Postprocessor::CaptureTarget::Plot : Postprocessor::CaptureTarget::View;
    data.get_string("capture_name",request.name);
    std::string file;
    bool valid = active && data.get_string("capture_file",file);
    if (data.find_keyword("WIDTH") || data.find_keyword("HEIGHT"))
    {
      double width = 0, height = 0;
      valid = valid && data.get_value("capture_width",width) && data.get_value("capture_height",height) &&
        std::isfinite(width) && std::isfinite(height) && std::floor(width) == width && std::floor(height) == height &&
        width >= 64 && width <= 8192 && height >= 64 && height <= 8192;
      if (valid)
      {
        request.width = static_cast<int>(width);
        request.height = static_cast<int>(height);
      }
    }
    auto value = [&](const char* key,const char* label,std::string& text)
    {
      if (!data.find_keyword(key))
      {
        return false;
      }
      if (!data.get_string(label,text))
      {
        valid = false;
      }
      std::transform(text.begin(),text.end(),text.begin(),[](unsigned char c)
      {
        return static_cast<char>(std::tolower(c));
      });
      return true;
    };
    std::string text;
    if (value("BACKGROUND","capture_background",text))
    {
      valid = valid && (text == "current" || text == "white" || text == "transparent");
      request.background = text == "white" ? Postprocessor::CaptureBackground::White :
        text == "transparent" ? Postprocessor::CaptureBackground::Transparent : Postprocessor::CaptureBackground::Current;
    }
    const auto flag = [&](const char* key,const char* label,std::optional<bool>& target)
    {
      std::string setting;
      if (value(key,label,setting))
      {
        valid = valid && (setting == "on" || setting == "off");
        target = setting == "on";
      }
    };
    flag("LEGEND","capture_legend",request.legend);
    flag("LABELS","capture_labels",request.labels);
    flag("TRIAD","capture_triad",request.triad);
    flag("MARKER","capture_marker",request.marker);
    if (valid && active->capture_file(request,file,error))
    {
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  PRINT_ERROR("Cannot capture Postprocessor image: %s\n",error.empty() ? "Invalid capture arguments." : error.c_str());
  return false;
}
