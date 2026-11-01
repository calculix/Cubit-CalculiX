#ifndef CORETOPOLOGYINTERACTIONPRESERVATION_HPP
#define CORETOPOLOGYINTERACTIONPRESERVATION_HPP

#include <map>
#include <set>

class CoreConstraints;
class CoreContactPairs;

class CoreTopologyInteractionPreservation
{
public:
  static bool append_constraints(CoreConstraints& constraints, std::map<int, std::set<int>>& sets, std::set<int>& nodes);
  static bool append_contact_pairs(CoreContactPairs& contacts, std::map<int, std::set<int>>& sets);
};

#endif
