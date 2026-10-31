#ifndef CORECONTACTPAIRSCLEANUP_HPP
#define CORECONTACTPAIRSCLEANUP_HPP

#include <string>

class CoreContactPairs;
class CoreSurfaceInteractions;
class MeshExportInterface;

class CoreContactPairsCleanup
{
public:
  static bool cleanup(CoreContactPairs& contacts, CoreSurfaceInteractions& interactions, MeshExportInterface& mesh, std::string& log);
};

#endif
