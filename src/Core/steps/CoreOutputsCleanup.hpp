#ifndef COREOUTPUTSCLEANUP_HPP
#define COREOUTPUTSCLEANUP_HPP

#include <string>

class CoreHistoryOutputs;
class CoreFieldOutputs;
class CoreBlocks;
class CoreContactPairs;
class MeshExportInterface;

class CoreOutputsCleanup
{
public:
  static bool cleanup(CoreHistoryOutputs& outputs, CoreBlocks& blocks, CoreContactPairs& contacts, MeshExportInterface& mesh, std::string& log);
  static bool cleanup(CoreFieldOutputs& outputs, CoreBlocks& blocks, CoreContactPairs& contacts, MeshExportInterface& mesh, std::string& log);
};

#endif
