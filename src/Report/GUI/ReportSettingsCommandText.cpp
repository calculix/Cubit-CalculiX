#include "ReportSettingsCommandText.hpp"
#include "ReportCommandText.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include <map>
#include <utility>
#include <QRegularExpression>

bool ReportSettingsCommandText::build(const ReportConfiguration& patch, const QString& logoFile,
  std::string& output, std::string& error, const ReportTemplate* definition)
{
  std::map<std::string, std::string> kinds;
  for (const auto& option : ReportUserOptions::definitions())
  {
    kinds.emplace(option.key, option.kind);
  }
  kinds.emplace("steps", "text");
  std::string command = "report configure settings";
  std::string templateArguments;
  for (const auto& entry : patch.values)
  {
    const auto kind = kinds.find(entry.first);
    if (kind == kinds.end())
    {
      bool known = false;
      if (definition != nullptr)
      {
        for (const auto& option : definition->options)
        {
          known = known || option.key == entry.first;
        }
      }
      std::string key, value;
      if (!known)
      {
        error = "Unknown report settings command argument: " + entry.first;
        return false;
      }
      if (!ReportCommandText::quote(QString::fromStdString(entry.first), key, error) ||
          !ReportCommandText::quote(QString::fromStdString(entry.second), value, error))
      {
        return false;
      }
      templateArguments += " option " + key + " value " + value;
      continue;
    }
    std::string value;
    if (kind->second == "boolean")
    {
      if (entry.second != "true" && entry.second != "false")
      {
        error = "Expected a boolean setting for " + entry.first;
        return false;
      }
      value = entry.second == "true" ? "on" : "off";
    }
    else if (kind->second == "precision" || kind->second == "logo_width" || kind->second == "logo_placement" || kind->second == "page_size")
    {
      if (!QRegularExpression("^[A-Za-z0-9_+-]+$").match(QString::fromStdString(entry.second)).hasMatch())
      {
        error = "Invalid command token for " + entry.first;
        return false;
      }
      value = entry.second;
    }
    else if (!ReportCommandText::quote(QString::fromStdString(entry.second), value, error))
    {
      return false;
    }
    command += " " + entry.first + " " + value;
  }
  command += templateArguments;
  if (patch.replace_company_logo)
  {
    if (logoFile.isEmpty())
    {
      if (patch.company_logo)
      {
        error = "Importing a new company logo requires its filename.";
        return false;
      }
      command += " logo_remove";
    }
    else
    {
      std::string quoted;
      if (!ReportCommandText::quote(logoFile, quoted, error))
      {
        return false;
      }
      command += " logo_file " + quoted;
    }
  }
  if (patch.values.empty() && !patch.replace_company_logo)
  {
    error = "No report settings changed.";
    return false;
  }
  output = std::move(command);
  error.clear();
  return true;
}
