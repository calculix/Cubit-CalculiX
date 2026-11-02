#ifndef PERSISTENCETABLE_HPP
#define PERSISTENCETABLE_HPP

#include <charconv>
#include <cstddef>
#include <system_error>
#include <cmath>
#include <initializer_list>
#include <limits>
#include <set>
#include <string>
#include <vector>

// Data checks only; error handling and feature transactions stay in each persistence class.
namespace PersistenceTable
{
inline bool id(const std::string& value, int& result)
{
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
  return parsed.ec == std::errc() && parsed.ptr == value.data() + value.size();
}

template<class T>
bool id(T value, int& result)
{
  if (!std::isfinite(static_cast<double>(value)) || value < (std::numeric_limits<int>::min)() || value > (std::numeric_limits<int>::max)())
  {
    return false;
  }
  result = static_cast<int>(value);
  return value == static_cast<T>(result);
}

template<class T>
bool rows(const std::vector<std::vector<T>>& table, std::size_t columns)
{
  for (const auto& row : table)
  {
    if (row.size() < columns)
    {
      return false;
    }
  }
  return true;
}

template<class T>
bool keyedRows(const std::vector<std::vector<T>>& table, std::size_t columns)
{
  if (!rows(table, columns))
  {
    return false;
  }
  for (const auto& row : table)
  {
    int key = 0;
    if (row.empty() || !id(row[0], key) || key < 0)
    {
      return false;
    }
  }
  return true;
}

template<class T>
bool contains(const std::vector<std::vector<T>>& table, int key)
{
  for (const auto& row : table)
  {
    int value = 0;
    if (!row.empty() && id(row[0], value) && value == key)
    {
      return true;
    }
  }
  return false;
}

template<class T>
bool uniqueIds(const std::vector<std::vector<T>>& table)
{
  std::set<int> seen;
  for (const auto& row : table)
  {
    int key = 0;
    if (row.empty() || !id(row[0], key) || key <= 0 || !seen.insert(key).second)
    {
      return false;
    }
  }
  return true;
}

template<class T, class U>
bool references(const std::vector<std::vector<T>>& table, std::size_t column, const std::vector<std::vector<U>>& detail)
{
  std::set<int> keys;
  for (const auto& row : detail)
  {
    int key = 0;
    if (row.empty() || !id(row[0], key))
    {
      return false;
    }
    keys.insert(key);
  }
  for (const auto& row : table)
  {
    int key = 0;
    if (row.size() <= column || !id(row[column], key) || (key >= 0 && keys.count(key) == 0))
    {
      return false;
    }
  }
  return true;
}

template<class T, class U>
bool selectedReferences(const std::vector<std::vector<T>>& table, std::size_t typeColumn, int type, std::size_t referenceColumn, const std::vector<std::vector<U>>& detail)
{
  std::set<int> keys;
  for (const auto& row : detail)
  {
    int key = 0;
    if (row.empty() || !id(row[0], key))
    {
      return false;
    }
    keys.insert(key);
  }
  for (const auto& row : table)
  {
    int actualType = 0;
    int key = 0;
    if (row.size() <= typeColumn || !id(row[typeColumn], actualType))
    {
      return false;
    }
    if (actualType == type && (row.size() <= referenceColumn || !id(row[referenceColumn], key) || (key >= 0 && keys.count(key) == 0)))
    {
      return false;
    }
  }
  return true;
}

template<class T>
bool types(const std::vector<std::vector<T>>& table, std::size_t column, std::initializer_list<int> allowed)
{
  const std::set<int> values(allowed);
  for (const auto& row : table)
  {
    int value = 0;
    if (row.size() <= column || !id(row[column], value) || values.count(value) == 0)
    {
      return false;
    }
  }
  return true;
}
}

#endif
