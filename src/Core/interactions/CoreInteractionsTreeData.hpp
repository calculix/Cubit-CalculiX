#ifndef COREINTERACTIONSTREEDATA_HPP
#define COREINTERACTIONSTREEDATA_HPP

#include <string>
#include <vector>

class CoreConstraints;
class CoreSurfaceInteractions;
class CoreContactPairs;

class CoreInteractionsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_constraints_tree_data(CoreConstraints& constraints);
  static std::vector<std::vector<std::string>> get_surfaceinteractions_tree_data(CoreSurfaceInteractions& interactions);
  static std::vector<std::vector<std::string>> get_contactpairs_tree_data(CoreContactPairs& pairs);
};

#endif
