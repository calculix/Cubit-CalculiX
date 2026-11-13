#include "ReportConfigurationResolver.hpp"
#include <utility>
#include "ReportUserOptions.hpp"
#include "ReportLogo.hpp"
#include "Report/Document/ReportPageDecoration.hpp"
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <set>

ReportConfigurationResolver::ReportConfigurationResolver() = default;

void ReportConfigurationResolver::add_common_options(ReportTemplate& definition)
{
  for (const auto& option : ReportUserOptions::definitions())
  {
    definition.options.push_back(option);
    definition.defaults.values[option.key] = option.default_value;
  }
}

bool ReportConfigurationResolver::resolve(const ReportTemplate& definition, const ReportConfiguration& overrides,
  ReportConfiguration& output, std::string& error)
{
  auto candidate = definition.defaults;
  candidate.company_logo = overrides.company_logo;
  std::set<std::string> keys;
  for (const auto& option : definition.options)
  {
    if (option.key.empty() || !keys.insert(option.key).second)
    {
      error = "Report template contains an empty or duplicate option key.";
      return false;
    }
    if (candidate.values.count(option.key) == 0)
    {
      candidate.values[option.key] = option.default_value;
    }
  }
  for (const auto& entry : overrides.values)
  {
    candidate.values[entry.first] = entry.second;
  }
  for (const auto& entry : candidate.values)
  {
    const auto found = std::find_if(definition.options.begin(), definition.options.end(), [&](const ReportOption& option)
    {
      return option.key == entry.first;
    });
    if (found == definition.options.end())
    {
      error = "Unknown report option: " + entry.first;
      return false;
    }
    const QString value = QString::fromStdString(entry.second);
    bool valid = value.size() <= 4096 && !value.contains(QChar('\0'));
    if (found->kind == "boolean")
    {
      valid = valid && (value == "true" || value == "false");
    }
    else if (found->kind == "precision")
    {
      bool ok = false;
      const int digits = value.toInt(&ok);
      valid = valid && ok && digits >= 2 && digits <= 12;
    }
    else if (found->kind == "integer")
    {
      bool ok = false;
      const int number = value.toInt(&ok);
      valid = valid && ok && found->minimum <= found->maximum && number >= found->minimum && number <= found->maximum;
    }
    else if (found->kind == "choice")
    {
      valid = valid && !found->choices.empty() && std::find(found->choices.begin(), found->choices.end(), entry.second) != found->choices.end();
    }
    else if (found->kind == "page_size")
    {
      valid = valid && (value == "A4" || value == "Letter");
    }
    else if (found->kind == "steps")
    {
      valid = valid && (value == "all" || QRegularExpression("^[1-9][0-9]*(,[1-9][0-9]*)*$").match(value).hasMatch());
    }
    else if (found->kind == "decoration")
    {
      valid = valid && value.size() <= 160 && !value.contains('\n') && !value.contains('\r');
    }
    else if (found->kind == "logo_placement")
    {
      valid = valid && QRegularExpression("^(header|footer)_(left|center|right)$").match(value).hasMatch();
    }
    else if (found->kind == "logo_width")
    {
      bool ok = false;
      const int width = value.toInt(&ok);
      valid = valid && ok && width >= 5 && width <= 60;
    }
    else if (found->kind != "text")
    {
      valid = false;
    }
    if (entry.first == "title" && value.trimmed().isEmpty())
    {
      valid = false;
    }
    if (entry.first == "company_name" || entry.first == "company_details")
    {
      valid = valid && value.size() <= 256 && !value.contains('\n') && !value.contains('\r');
    }
    if (!valid)
    {
      error = "Invalid report option value: " + entry.first;
      return false;
    }
  }
  if (!ReportPageDecoration::validate_templates(candidate, error) ||
      (candidate.company_logo && !ReportLogo::validate(*candidate.company_logo, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
