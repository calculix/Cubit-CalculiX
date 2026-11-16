#include "ReportConfigurationResolver.hpp"
#include "ReportUnitValidation.hpp"
#include <utility>
#include "ReportUserOptions.hpp"
#include "ReportLogo.hpp"
#include "Report/Document/ReportPageDecoration.hpp"
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>
#include <set>
#include <map>

ReportConfigurationResolver::ReportConfigurationResolver() = default;

void ReportConfigurationResolver::add_common_options(ReportTemplate& definition)
{
  definition.content_groups.insert(definition.content_groups.begin(), {
    {"report.content.document", "Document"},
    {"report.content.decoration", "Header and footer", "report.content.document"}});
  for (const auto& option : ReportUserOptions::definitions())
  {
    definition.options.push_back(option);
    definition.defaults.values[option.key] = option.default_value;
  }
}

bool ReportConfigurationResolver::resolve(const ReportTemplate& definition, const ReportConfiguration& overrides,
  ReportConfiguration& output, std::string& error)
{
  std::map<std::string, const ReportContentGroup*> groups;
  std::set<std::string> gates;
  for (const auto& group : definition.content_groups)
  {
    if (group.id.empty() || group.label.empty() || !groups.emplace(group.id, &group).second)
    {
      error = "Report content groups require unique IDs and labels.";
      return false;
    }
  }
  for (const auto& group : definition.content_groups)
  {
    std::set<std::string> ancestors;
    const ReportContentGroup* current = &group;
    while (current != nullptr)
    {
      if (!ancestors.insert(current->id).second)
      {
        error = "Report content groups contain a cycle.";
        return false;
      }
      if (current->parent_id.empty())
      {
        break;
      }
      const auto parent = groups.find(current->parent_id);
      if (parent == groups.end())
      {
        error = "Report content group parent is unavailable.";
        return false;
      }
      current = parent->second;
    }
    if (!group.gate_option.empty())
    {
      if (!gates.insert(group.gate_option).second)
      {
        error = "Report content groups must not share a section gate.";
        return false;
      }
      const auto gate = std::find_if(definition.options.begin(), definition.options.end(), [&](const ReportOption& option)
      {
        return option.key == group.gate_option && option.kind == "boolean";
      });
      if (gate == definition.options.end())
      {
        error = "Report content group gate must reference a boolean option.";
        return false;
      }
    }
  }
  for (const auto& option : definition.options)
  {
    if (!option.group_id.empty() && groups.count(option.group_id) == 0)
    {
      error = "Report option refers to an unavailable content group: " + option.key;
      return false;
    }
  }
  auto candidate = definition.defaults;
  candidate.company_logo = overrides.company_logo;
  candidate.units = overrides.units;
  if (!ReportUnitValidation::configuration(candidate, error))
  {
    return false;
  }
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
