#include "ReportConfigureCommand.hpp"
#include <utility>
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <QJsonDocument>
#include <QJsonParseError>

ReportConfigureCommand::ReportConfigureCommand(std::weak_ptr<ReportApplicationInterface> active) : application(std::move(active))
{
}

ReportConfigureCommand::~ReportConfigureCommand() = default;

std::vector<std::string> ReportConfigureCommand::get_syntax()
{
  return {"report configure <string:type='quoted',number='1',label='key'> <string:type='quoted',number='1',label='value'>",
    "report configure settings data <string:type='quoted',number='1',label='settings_data'>",
    "report configure logo file <string:type='quoted',number='1',label='logo_filename'>",
    "report configure logo remove",
    "report configure logo placement {header|footer} {left|center|right} width <value:label='logo_width'>",
    "report configure settings title <string:type='quoted',number='1',label='title'> author <string:type='quoted',number='1',label='author'> project <string:type='quoted',number='1',label='project'> revision <string:type='quoted',number='1',label='revision'> precision <value:label='precision'> paper {a4|letter} steps <string:type='quoted',number='1',label='steps'>",
    "report select <string:type='quoted',number='1',label='report_id'>",
    "report note <string:type='quoted',number='1',label='section_id'> <string:type='quoted',number='1',label='note_text'>"};
}

std::vector<std::string> ReportConfigureCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ReportConfigureCommand::get_help()
{
  return {"Configure a template option, select a saved report, or attach a note to a stable section ID. Update after configuration changes."};
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
  if (data.find_keyword("SETTINGS") && data.find_keyword("DATA"))
  {
    // Base64 transports JSON without Cubit's quote/control-character ambiguity.
    // Logo bytes stay out of the command: the Core imports the selected path.
    std::string encoded;
    if (!data.get_string("settings_data", encoded) || encoded.size() > 64 * 1024)
    {
      error = "Report settings data is missing or too large.";
      return false;
    }
    const auto bytes = QByteArray::fromBase64Encoding(QByteArray::fromStdString(encoded), QByteArray::AbortOnBase64DecodingErrors);
    QJsonParseError parse;
    const auto json = QJsonDocument::fromJson(bytes.decoded, &parse);
    ReportConfiguration patch;
    if (!bytes || parse.error != QJsonParseError::NoError || !json.isObject() ||
        !ReportConfiguration::from_json(json.object(), patch, error))
    {
      error = "Invalid report settings data.";
      return false;
    }
    const auto file = json.object().value("logo_file");
    if (!file.isUndefined())
    {
      ReportImage image;
      if (!file.isString() || !active->load_logo(file.toString().toStdString(), image, error))
      {
        return false;
      }
      patch.company_logo = std::move(image);
      patch.replace_company_logo = true;
    }
    return active->configure_patch(patch, error);
  }
  if (data.find_keyword("SETTINGS"))
  {
    ReportConfiguration patch;
    for (const std::string key : {"title", "author", "project", "revision", "steps"})
    {
      std::string value;
      if (!data.get_string(key, value))
      {
        error = "Incomplete report settings command.";
        return false;
      }
      patch.values[key] = value;
    }
    int precision = 0;
    if (!data.get_value("precision", precision))
    {
      error = "Report settings require a precision value.";
      return false;
    }
    patch.values["precision"] = std::to_string(precision);
    patch.values["page_size"] = data.find_keyword("LETTER") ? "Letter" : "A4";
    return active->configure_patch(patch, error);
  }
  std::string key, value;
  if (data.find_keyword("SELECT"))
  {
    return data.get_string("report_id", key) && active->select(key, error);
  }
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
