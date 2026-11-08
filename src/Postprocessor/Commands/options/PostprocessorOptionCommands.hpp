#ifndef POSTPROCESSOR_OPTIONCOMMANDS_HPP
#define POSTPROCESSOR_OPTIONCOMMANDS_HPP

#include "Framework/Commands/UserOptionCommandSupport.hpp"
#include "Postprocessor/Core/options/PostprocessorUserOptions.hpp"
#include <algorithm>

// Module-local RGB command transport. Preferences still use the existing typed
// store; one RGB command changes all three components atomically.
namespace PostprocessorOptionCommands
{
inline bool background(const std::string& key)
{
  return key == "background/top" || key == "background/bottom";
}

inline std::vector<UserOption> scalar_schema(const std::vector<UserOption>& schema)
{
  auto result = schema;
  result.erase(std::remove_if(result.begin(), result.end(), [](const UserOption& item)
  {
    return background(item.key);
  }), result.end());
  return result;
}

inline std::vector<std::string> syntax(const std::vector<UserOption>& schema)
{
  auto result = UserOptionCommandSupport::syntax("post useroptions", scalar_schema(schema));
  result.push_back("post useroptions background {top|bottom} <value:label='red'> <value:label='green'> <value:label='blue'>");
  return result;
}

template <typename Data>
bool read(Data& data, const std::vector<UserOption>& schema, UserOptionValues& changes, std::string& error)
{
  if (!data.find_keyword("BACKGROUND"))
  {
    return UserOptionCommandSupport::read(data, scalar_schema(schema), changes, error);
  }
  std::array<double, 3> color;
  if (!data.get_value("red", color[0]) || !data.get_value("green", color[1]) || !data.get_value("blue", color[2]))
  {
    error = "Background requires three RGB components.";
    return false;
  }
  const std::string key = data.find_keyword("TOP") ? "background/top" : "background/bottom";
  changes = {{key, Postprocessor::format_background_color(color)}};
  UserOptionValues candidate;
  return merge_user_option_changes(schema, changes, candidate, error);
}

inline bool format(const UserOption& item, const UserOptionValue& value, std::string& command, std::string& error)
{
  if (item.key == "mouse/left" || item.key == "mouse/middle" || item.key == "mouse/right")
  {
    UserOptionValues candidate;
    if (!merge_user_option_changes({item}, {{item.key, value}}, candidate, error))
    {
      return false;
    }
    command = "post useroptions mouse " + item.key.substr(6) + " " + std::get<std::string>(value);
    return true;
  }
  if (!background(item.key))
  {
    return UserOptionCommandSupport::format("post useroptions", item, value, command, error);
  }
  UserOptionValues candidate;
  if (!merge_user_option_changes({item}, {{item.key, value}}, candidate, error))
  {
    return false;
  }
  command = "post useroptions background " + item.key.substr(11) + " " + std::get<std::string>(value);
  return true;
}

inline bool edited_commands(const std::vector<UserOption>& schema, const QJsonObject& edited,
  const QJsonObject& baseline, std::vector<std::pair<std::string, std::string>>& commands, std::string& error)
{
  if (!UserOptionCommandSupport::edited_commands("post useroptions", scalar_schema(schema), edited, baseline, commands, error))
  {
    return false;
  }
  for (auto& entry : commands)
  {
    if (entry.first == "mouse/left" || entry.first == "mouse/middle" || entry.first == "mouse/right")
    {
      const auto found = std::find_if(schema.begin(), schema.end(), [&](const UserOption& item)
      {
        return item.key == entry.first;
      });
      if (found == schema.end() || !format(*found, edited.value(QString::fromStdString(entry.first)).toString().toStdString(), entry.second, error))
      {
        return false;
      }
    }
  }
  for (const auto& item : schema)
  {
    const auto key = QString::fromStdString(item.key);
    if (background(item.key) && edited.value(key) != baseline.value(key))
    {
      std::string command;
      if (!format(item, edited.value(key).toString().toStdString(), command, error))
      {
        return false;
      }
      commands.emplace_back(item.key, std::move(command));
    }
  }
  return true;
}
}
#endif
