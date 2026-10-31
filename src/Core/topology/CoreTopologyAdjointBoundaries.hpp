#ifndef CORETOPOLOGYADJOINTBOUNDARIES_HPP
#define CORETOPOLOGYADJOINTBOUNDARIES_HPP

#include <set>
#include <utility>

class CoreBCsDisplacements;
class CoreHBCs;
class CoreSteps;
class MeshExportInterface;

class CoreTopologyAdjointBoundaries
{
public:
  static bool collect(CoreBCsDisplacements& displacements, const CoreHBCs& hbcs, CoreSteps& steps, MeshExportInterface& mesh, std::set<std::pair<int, int>>& boundaries);
};

#endif
