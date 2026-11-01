#ifndef COREJOBINPUTPREPARATION_HPP
#define COREJOBINPUTPREPARATION_HPP

#include <functional>
#include <string>

class CoreTopology;
struct CoreJobExportContext;

class CoreJobInputPreparation
{
public:
  static bool prepare(const CoreTopology* topology, CoreJobExportContext& active_context, int job_id, const std::string& filename, const std::function<bool(const std::string&)>& execute_command);
};

#endif
