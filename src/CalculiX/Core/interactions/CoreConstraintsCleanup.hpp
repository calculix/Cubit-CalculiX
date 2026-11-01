#ifndef CORECONSTRAINTSCLEANUP_HPP
#define CORECONSTRAINTSCLEANUP_HPP

#include <string>

class CoreConstraints;
class CoreBlocks;
class MeshExportInterface;

class CoreConstraintsCleanup
{
public:
  static bool cleanup(CoreConstraints& constraints, CoreBlocks& blocks, MeshExportInterface& mesh, std::string& log);
};

#endif
