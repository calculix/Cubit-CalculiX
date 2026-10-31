#ifndef CORECUBITCONTACTIMPORT_HPP
#define CORECUBITCONTACTIMPORT_HPP

#include <string>

class CoreConstraints;
class CoreContactPairs;
class CoreSurfaceInteractions;
class MeshExportInterface;

class CoreCubitContactImport
{
public:
  static bool create_ties(CoreConstraints& constraints, MeshExportInterface& mesh, const std::string& name, const std::string& position_tolerance);
  static bool create_pairs(CoreContactPairs& pairs, CoreSurfaceInteractions& interactions, MeshExportInterface& mesh, int surfaceinteraction_id, const std::string& contactpairtype, const std::string& adjust, const std::string& adjust_nodeset);
};

#endif
