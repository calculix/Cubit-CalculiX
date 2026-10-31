#ifndef CORESECTIONSCLEANUP_HPP
#define CORESECTIONSCLEANUP_HPP

#include <string>

class CoreSections;
class CoreMaterials;
class CoreBlocks;
class CoreOrientations;

class CoreSectionsCleanup
{
public:
  static bool cleanup(CoreSections& sections, CoreMaterials& materials, CoreBlocks& blocks, CoreOrientations* orientations, std::string& log);
};

#endif
