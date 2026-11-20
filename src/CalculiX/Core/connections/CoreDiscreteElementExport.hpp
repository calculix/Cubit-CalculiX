#pragma once
#include <array>
#include <map>
#include <ostream>
#include <set>
#include <string>

class CoreConnections;
class CoreSteps;
class CoreExportNodeAllocator;

class CoreDiscreteElementExport
{
public:
  static bool append(const CoreConnections& model, CoreSteps& steps, CoreExportNodeAllocator& allocator,
    const std::set<int>& mesh_nodes, const std::map<int,std::array<int,2>>& roles,
    const std::map<int,std::array<double,3>>& positions, std::ostream& nodes, std::ostream& elements, std::string& error);
  static bool needs_history(const CoreConnections& model, int step_id, int procedure);
  static std::string history_set(int ordinal);
};
