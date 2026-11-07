#ifndef CUBITSIM_USEROPTION_HPP
#define CUBITSIM_USEROPTION_HPP

#include <functional>
#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

enum class UserOptionKind
{
  Boolean,
  Integer,
  Real,
  Text,
  ComponentPath
};

using UserOptionValue = std::variant<bool, int, double, std::string>;
using UserOptionValues = std::map<std::string, UserOptionValue>;

// No default constructor: every registered item requires an explicit default.
// ComponentPath strings are absolute or relative to the component directory.
// Keys are slash-separated identifiers, independent of translated labels.
struct UserOption
{
  UserOption(std::string key, UserOptionKind kind, UserOptionValue defaultValue,
    std::function<bool(const UserOptionValue&)> validator = {})
    : key(std::move(key)), kind(kind), defaultValue(std::move(defaultValue)),
      validator(std::move(validator))
  {
  }

  std::string key;
  UserOptionKind kind;
  UserOptionValue defaultValue;
  std::function<bool(const UserOptionValue&)> validator;
};

// Validate the entire patch before changing the candidate snapshot. Defaults,
// types and workspace-specific validators remain authoritative in Core.
inline bool merge_user_option_changes(const std::vector<UserOption>& schema,
  const UserOptionValues& changes, UserOptionValues& values, std::string& error)
{
  for (const auto& change : changes)
  {
    const auto definition = std::find_if(schema.begin(), schema.end(), [&](const UserOption& item)
    {
      return item.key == change.first;
    });
    if (definition == schema.end() || definition->defaultValue.index() != change.second.index() ||
      (definition->validator && !definition->validator(change.second)))
    {
      error = "Invalid user option: " + change.first;
      return false;
    }
  }
  for (const auto& change : changes)
  {
    values[change.first] = change.second;
  }
  return true;
}

#endif
