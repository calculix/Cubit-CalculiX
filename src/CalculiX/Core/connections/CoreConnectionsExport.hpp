#pragma once
#include <string>
#include <vector>

class CoreConnections;
class CoreSteps;
class CoreExportNodeAllocator;
class CoreConnectionsExport
{
public:
  static bool prepare(const CoreConnections& connections, CoreSteps& steps, const std::vector<int>& mesh_nodes, CoreExportNodeAllocator& allocator,
    std::string& nodes, std::string& elements, std::string& constraints, std::string& error);
};
