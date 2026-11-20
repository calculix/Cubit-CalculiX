#pragma once
#include <ostream>
#include <set>
#include <string>
class CoreConnections;
class CoreExportNodeAllocator;
class CoreDistributingExport
{
public:
  static bool append(const CoreConnections& model, CoreExportNodeAllocator& allocator, const std::set<int>& rigid,
    std::ostream& nodes, std::ostream& cards, std::string& error);
};
