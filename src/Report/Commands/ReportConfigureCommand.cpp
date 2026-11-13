#include "ReportConfigureCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include "Report/Core/ReportUserOptions.hpp"
#include <QString>
#include <map>
#include <cmath>
#include <limits>

namespace
{
std::map<std::string, std::string> settingsOptions()
{
  std::map<std::string, std::string> options;
  for (const auto& option : ReportUserOptions::definitions())
  {
    options.emplace(option.key, option.kind);
  }
  options.emplace("steps", "text");
  return options;
}
}

ReportConfigureCommand::ReportConfigureCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportConfigureCommand::~ReportConfigureCommand() = default;

std::vector<std::string> ReportConfigureCommand::get_syntax()
{
  std::string settings = "report configure settings";
  for (const auto& option : settingsOptions())
  {
    const bool numeric = option.second == "precision" || option.second == "logo_width";
    const bool word = option.second == "boolean" || option.second == "page_size" || option.second == "logo_placement";
    const std::string argument = numeric ? "<value:" : word ? "<string:type='unquoted',number='1'," : "<string:type='quoted',number='1',";
    settings += " [" + option.first + " " + argument + "label='" + option.first + "'>]";
  }
  settings += " [option <string:type='quoted',number='1',label='option_key'> value <string:type='quoted',number='1',label='option_value'>]...";
  settings += " [logo_file <string:type='quoted',number='1',label='logo_file'>] [logo_remove]";
  return {"report configure <string:type='quoted',number='1',label='key'> <string:type='quoted',number='1',label='value'>",
    settings,
    "report configure logo file <string:type='quoted',number='1',label='logo_filename'>",
    "report configure logo remove",
    "report configure logo placement {header|footer} {left|center|right} width <value:label='logo_width'>",
    "report note <string:type='quoted',number='1',label='section_id'> <string:type='quoted',number='1',label='note_text'>"};
}

std::vector<std::string> ReportConfigureCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportConfigureCommand::get_help()
{
  return {"Configure a template option or attach a note to a stable section ID. Update after configuration changes."};
}

bool ReportConfigureCommand::execute(CubitCommandData& data)
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

bool ReportConfigureCommand::run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error)
{
  if (data.find_keyword("LOGO"))
  {
    ReportConfiguration patch;
    if (data.find_keyword("PLACEMENT"))
    {
      int width = 0;
      if (!data.get_value("logo_width", width))
      {
        error = "Logo maximum width requires an integer in millimetres.";
        return false;
      }
      patch.values["logo_width_mm"] = std::to_string(width);
      patch.values["logo_placement"] = std::string(data.find_keyword("HEADER") ? "header_" : "footer_") +
        (data.find_keyword("LEFT") ? "left" : data.find_keyword("CENTER") ? "center" : "right");
    }
    else
    {
      patch.replace_company_logo = true;
      if (!data.find_keyword("REMOVE"))
      {
        std::string filename;
        ReportImage image;
        if (!data.get_string("logo_filename", filename) || !active->load_logo(filename, image, error))
        {
          return false;
        }
        patch.company_logo = std::move(image);
      }
    }
    return active->configure_patch(patch, error);
  }
  if (data.find_keyword("SETTINGS"))
  {
    ReportConfiguration patch;
    if (data.find_keyword("OPTION"))
    {
      std::vector<std::string> keys, values;
      if (!data.get_strings("option_key", keys) || !data.get_strings("option_value", values) || keys.empty() || keys.size() != values.size())
      {
        error = "Each report setting option requires one quoted key and value.";
        return false;
      }
      for (std::size_t index = 0; index < keys.size(); ++index)
      {
        if (settingsOptions().count(keys[index]) != 0 || !patch.values.emplace(keys[index], values[index]).second)
        {
          error = "Duplicate setting or common option used through template-option syntax: " + keys[index];
          return false;
        }
      }
    }
    for (const auto& option : settingsOptions())
    {
      const auto keyword = QString::fromStdString(option.first).toUpper().toStdString();
      if (!data.find_keyword(keyword.c_str()))
      {
        continue;
      }
      std::string value;
      if (option.second == "precision" || option.second == "logo_width")
      {
        double number = 0;
        if (!data.get_value(option.first.c_str(), number) || !std::isfinite(number) || std::trunc(number) != number ||
            number < (std::numeric_limits<int>::min)() || number > (std::numeric_limits<int>::max)())
        {
          error = "Expected an integer for " + option.first + ".";
          return false;
        }
        value = std::to_string(static_cast<int>(number));
      }
      else if (!data.get_string(option.first.c_str(), value))
      {
        error = "Missing report setting: " + option.first;
        return false;
      }
      if (option.second == "boolean")
      {
        const auto flag = QString::fromStdString(value).toLower();
        if (flag != "on" && flag != "off")
        {
          error = "Expected on or off for " + option.first + ".";
          return false;
        }
        value = flag == "on" ? "true" : "false";
      }
      else if (option.second == "page_size")
      {
        const auto paper = QString::fromStdString(value);
        if (paper.compare("A4", Qt::CaseInsensitive) == 0)
        {
          value = "A4";
        }
        else if (paper.compare("Letter", Qt::CaseInsensitive) == 0)
        {
          value = "Letter";
        }
      }
      else if (option.second == "logo_placement")
      {
        value = QString::fromStdString(value).toLower().toStdString();
      }
      patch.values[option.first] = value;
    }
    const bool importLogo = data.find_keyword("LOGO_FILE");
    const bool removeLogo = data.find_keyword("LOGO_REMOVE");
    if (importLogo && removeLogo)
    {
      error = "Choose either logo_file or logo_remove.";
      return false;
    }
    patch.replace_company_logo = importLogo || removeLogo;
    if (importLogo)
    {
      std::string filename;
      ReportImage image;
      if (!data.get_string("logo_file", filename) || !active->load_logo(filename, image, error))
      {
        return false;
      }
      patch.company_logo = std::move(image);
    }
    if (patch.values.empty() && !patch.replace_company_logo)
    {
      error = "Specify at least one named report setting.";
      return false;
    }
    return active->configure_patch(patch, error);
  }
  std::string key, value;
  if (data.find_keyword("NOTE"))
  {
    return data.get_string("section_id", key) && data.get_string("note_text", value) && active->note(key, value, error);
  }
  if (!data.get_string("key", key) || !data.get_string("value", value))
  {
    error = "Report configuration needs quoted key and value strings.";
    return false;
  }
  return active->configure(key, value, error);
}
