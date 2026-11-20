#pragma once
#include <set>
#include <string>
#include <vector>

class CoreExportNodeAllocator
{
public:
  bool prepare(const std::vector<int>& mesh_nodes, const std::vector<std::vector<std::string>>& custom_lines,
    const std::string& filename, bool discrete_elements, std::string& error);
  bool reserve_set(const std::string& name);
  bool next(int& node);
  bool next_element(int& element);
  bool custom_element_uses(int node) const;
private:
  int highest_ = 0;
  int highest_element_ = 0;
  std::set<int> custom_element_nodes_;
  std::set<std::string> sets_;
};
