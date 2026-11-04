#ifndef FEA_USEROPTION_HPP
#define FEA_USEROPTION_HPP

#include <functional>
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

#endif
