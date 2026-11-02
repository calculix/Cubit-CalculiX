#ifndef CALCULIX_SETTINGS_READ_HPP
#define CALCULIX_SETTINGS_READ_HPP

#include <iomanip>
#include <cmath>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

// Storage validation only. Each feature defines and fills its own settings type.
namespace SettingsRead
{
inline std::string text(const std::string& value)
{
  return value;
}

template<typename Value>
std::string text(Value value)
{
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  stream << std::setprecision(17) << value;
  return stream.str();
}

template<typename Value>
const std::vector<Value>* row(const std::vector<std::vector<Value>>& rows, int id, std::size_t columns, std::string& error)
{
  const std::string key = std::to_string(id);

  for (const auto& candidate : rows)
  {
    if (!candidate.empty() && text(candidate[0]) == key)
    {
      if (candidate.size() >= columns)
      {
        return &candidate;
      }

      error = "Stored record " + key + " has incomplete data.";
      return nullptr;
    }
  }

  error = "Stored record " + key + " was not found.";
  return nullptr;
}

template<typename Value>
std::string field(const std::vector<std::vector<Value>>& rows, int id, std::size_t column, std::string& error)
{
  if (id < 0)
  {
    return {};
  }

  const auto* values = row(rows, id, column + 1, error);
  return values == nullptr ? std::string() : text((*values)[column]);
}

inline std::string optional_id(int id)
{
  return id < 0 ? std::string() : std::to_string(id);
}

template<typename Value>
std::vector<std::vector<double>> table(const std::vector<std::vector<Value>>& rows, int id, std::size_t columns, std::string& error)
{
  std::vector<std::vector<double>> result;

  if (id < 0)
  {
    return result;
  }

  for (const auto& entry : rows)
  {
    if (entry.empty() || text(entry[0]) != std::to_string(id))
    {
      continue;
    }

    if (entry.size() < columns + 1)
    {
      error = "Stored table has an incomplete row.";
      return {};
    }

    std::vector<double> values;
    for (std::size_t column = 1; column <= columns; ++column)
    {
      std::istringstream stream(text(entry[column]));
      stream.imbue(std::locale::classic());
      double value = 0;
      stream >> value;
      if (!stream || !std::isfinite(value))
      {
        error = "Stored table has an invalid numeric value.";
        return {};
      }
      stream >> std::ws;
      if (!stream.eof())
      {
        error = "Stored table has an invalid numeric suffix.";
        return {};
      }
      values.push_back(value);
    }
    result.push_back(values);
  }

  return result;
}
}

#endif // CALCULIX_SETTINGS_READ_HPP
