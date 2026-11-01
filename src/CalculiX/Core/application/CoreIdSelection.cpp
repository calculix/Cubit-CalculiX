#include "CoreIdSelection.hpp"

#include <algorithm>
#include <sstream>

std::vector<int> CoreIdSelection::extract_integers(const std::string& text)
{
  std::stringstream stream(text);
  std::vector<int> ids;
  std::string token;

  while (stream >> token)
  {
    std::stringstream number(token);
    int id;

    if (number >> id)
    {
      ids.push_back(id);
    }
  }

  return ids;
}

std::vector<int> CoreIdSelection::collect_ids(const std::vector<std::vector<int>>& rows)
{
  std::vector<int> ids;
  ids.reserve(rows.size());

  for (const auto& row : rows)
  {
    ids.push_back(row[0]);
  }

  return ids;
}

std::vector<int> CoreIdSelection::collect_ids(const std::vector<std::vector<std::string>>& rows)
{
  std::vector<int> ids;
  ids.reserve(rows.size());

  for (const auto& row : rows)
  {
    ids.push_back(std::stoi(row[0]));
  }

  return ids;
}

std::vector<int> CoreIdSelection::parse(const std::string& text, std::vector<int> all_ids)
{
  const bool has_all = text.find("all") != std::string::npos;
  const bool has_except = text.find("except") != std::string::npos;
  const bool has_range = text.find("to") != std::string::npos;
  const auto input_ids = extract_integers(text);

  if (!has_all && !has_except && !has_range)
  {
    return input_ids;
  }

  std::vector<int> range_ids;

  if (has_range)
  {
    if (input_ids.size() < 2)
    {
      return {};
    }

    if (input_ids[0] > 0 && input_ids[0] <= input_ids[1])
    {
      int id = input_ids[0];

      for (;;)
      {
        range_ids.push_back(id);

        if (id == input_ids[1])
        {
          break;
        }

        ++id;
      }
    }
  }

  if (!has_all && has_range)
  {
    return range_ids;
  }

  if (has_except)
  {
    const auto& excluded_ids = has_range ? range_ids : input_ids;

    for (int id : excluded_ids)
    {
      all_ids.erase(std::remove(all_ids.begin(), all_ids.end(), id), all_ids.end());
    }
  }

  return all_ids;
}
