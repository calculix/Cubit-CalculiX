#ifndef COREIDSELECTION_HPP
#define COREIDSELECTION_HPP

#include <string>
#include <vector>

class CoreIdSelection
{
public:
  static std::vector<int> extract_integers(const std::string& text);
  static std::vector<int> collect_ids(const std::vector<std::vector<int>>& rows);
  static std::vector<int> collect_ids(const std::vector<std::vector<std::string>>& rows);
  static std::vector<int> parse(const std::string& text, std::vector<int> all_ids);
};

#endif
