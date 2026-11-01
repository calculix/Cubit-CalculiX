#ifndef CORETOPOLOGYEXPORTVALIDATION_HPP
#define CORETOPOLOGYEXPORTVALIDATION_HPP

#include <functional>

struct CoreJobExportContext;
class CoreCustomLines;

class CoreTopologyExportValidation
{
public:
  static bool check(const CoreJobExportContext& context, CoreCustomLines* customlines, int topology_id, int iteration_id, const std::function<bool()>& validate_model);
};

#endif
