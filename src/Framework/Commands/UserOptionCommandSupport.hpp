#ifndef CUBITSIM_USEROPTIONCOMMANDSUPPORT_HPP
#define CUBITSIM_USEROPTIONCOMMANDSUPPORT_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include <QJsonObject>
#include <cmath>
#include <limits>
#include <locale>
#include <sstream>

// Shared textual transport only. Workspace Core schemas own defaults and validation.
namespace UserOptionCommandSupport
{
inline std::string name(const std::string& key)
{
  const auto slash = key.find('/');
  auto leaf = key.substr(slash + 1);
  if (key.substr(0, slash) == "monitor")
  {
    if (leaf == "enabled")
    {
      return "monitor";
    }
    if (leaf == "interval_seconds")
    {
      leaf = "interval";
    }
    if (leaf.find("show_") == 0)
    {
      leaf.erase(0, 5);
    }
    return "monitor " + leaf;
  }
  if (key.substr(0, slash) == "drawing")
  {
    return "drawing " + (leaf == "overlay_visible" ? std::string("overlay") : leaf);
  }
  return leaf;
}

inline std::vector<std::string> syntax(const std::string& prefix, const std::vector<UserOption>& schema)
{
  std::vector<std::string> result{prefix + " list"};
  for (std::size_t i = 0; i < schema.size(); ++i)
  {
    const auto kind = schema[i].kind;
    const auto argument = kind == UserOptionKind::Integer || kind == UserOptionKind::Real ?
      "<value:" : "<string:type='unquoted',number='1',";
    result.push_back(prefix + " " + name(schema[i].key) + " " + argument +
      "label='option_" + std::to_string(i) + "',help='<value>'>");
  }
  return result;
}

template <typename Data>
inline bool read(Data& data, const std::vector<UserOption>& schema,
  UserOptionValues& changes, std::string& error)
{
  changes.clear();
  error.clear();
  for (std::size_t i = 0; i < schema.size(); ++i)
  {
    const auto label = "option_" + std::to_string(i);
    const auto& item = schema[i];
    if (item.kind == UserOptionKind::Integer || item.kind == UserOptionKind::Real)
    {
      double value = 0;
      if (!data.get_value(label.c_str(), value))
      {
        continue;
      }
      if (item.kind == UserOptionKind::Integer)
      {
        if (!std::isfinite(value) || std::trunc(value) != value ||
          value < (std::numeric_limits<int>::min)() || value > (std::numeric_limits<int>::max)())
        {
          error = "Expected an integer for " + name(item.key);
          return false;
        }
        changes[item.key] = static_cast<int>(value);
      }
      else
      {
        changes[item.key] = value;
      }
    }
    else
    {
      std::string value;
      if (!data.get_string(label.c_str(), value))
      {
        continue;
      }
      if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
      {
        value = value.substr(1, value.size() - 2);
      }
      if (item.kind == UserOptionKind::Boolean)
      {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
        {
          return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : static_cast<char>(c);
        });
        if (value != "on" && value != "off")
        {
          error = "Expected on or off for " + name(item.key);
          return false;
        }
        changes[item.key] = value == "on";
      }
      else
      {
        changes[item.key] = value;
      }
    }
  }
  if (changes.size() != 1)
  {
    error = "Expected exactly one user-option setting.";
    return false;
  }
  UserOptionValues candidate;
  return merge_user_option_changes(schema, changes, candidate, error);
}

inline bool format(const std::string& prefix, const UserOption& item,
  const UserOptionValue& value, std::string& command, std::string& error)
{
  error.clear();
  UserOptionValues candidate;
  if (!merge_user_option_changes({item}, {{item.key, value}}, candidate, error))
  {
    return false;
  }
  std::ostringstream text;
  text.imbue(std::locale::classic());
  text.precision(std::numeric_limits<double>::max_digits10);
  text << prefix << ' ' << name(item.key) << ' ';
  if (const auto* flag = std::get_if<bool>(&value))
  {
    text << (*flag ? "on" : "off");
  }
  else if (const auto* integer = std::get_if<int>(&value))
  {
    text << *integer;
  }
  else if (const auto* real = std::get_if<double>(&value))
  {
    text << *real;
  }
  else
  {
    auto path = std::get<std::string>(value);
    if (path.find_first_of("\"\r\n") != std::string::npos || path.find('\0') != std::string::npos)
    {
      error = "Quotes and line breaks are not supported in user-option strings.";
      return false;
    }
#ifdef _WIN32
    if (item.kind == UserOptionKind::ComponentPath)
    {
      std::replace(path.begin(), path.end(), '\\', '/');
    }
#endif
    text << '"' << path << '"';
  }
  command = text.str();
  return true;
}

// Validate/format all edited values before submitting the first GUI command.
inline bool edited_commands(const std::string& prefix, const std::vector<UserOption>& schema,
  const QJsonObject& edited, const QJsonObject& baseline,
  std::vector<std::pair<std::string, std::string>>& commands, std::string& error)
{
  commands.clear();
  error.clear();
  for (const auto& item : schema)
  {
    const auto key = QString::fromStdString(item.key);
    const auto json = edited.value(key);
    if (json == baseline.value(key))
    {
      continue;
    }
    UserOptionValue value;
    const bool string = item.kind == UserOptionKind::Text || item.kind == UserOptionKind::ComponentPath;
    if ((item.kind == UserOptionKind::Boolean && !json.isBool()) || (string && !json.isString()) ||
      ((item.kind == UserOptionKind::Integer || item.kind == UserOptionKind::Real) &&
        (!json.isDouble() || !std::isfinite(json.toDouble()))) ||
      (item.kind == UserOptionKind::Integer && (std::trunc(json.toDouble()) != json.toDouble() ||
        json.toDouble() < (std::numeric_limits<int>::min)() || json.toDouble() > (std::numeric_limits<int>::max)())))
    {
      error = "Wrong type for user option: " + item.key;
      return false;
    }
    if (item.kind == UserOptionKind::Boolean)
    {
      value = json.toBool();
    }
    else if (item.kind == UserOptionKind::Integer)
    {
      value = json.toInt();
    }
    else if (item.kind == UserOptionKind::Real)
    {
      value = json.toDouble();
    }
    else
    {
      value = json.toString().toUtf8().toStdString();
    }
    std::string command;
    if (!format(prefix, item, value, command, error))
    {
      return false;
    }
    commands.emplace_back(item.key, std::move(command));
  }
  return true;
}
}

#endif
