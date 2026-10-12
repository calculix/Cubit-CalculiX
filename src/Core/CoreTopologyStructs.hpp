#ifndef CORETOPOLOGYSTRUCTS_HPP
#define CORETOPOLOGYSTRUCTS_HPP

#include <map>
#include <string>
#include <vector>
#include <set>

struct TopologyCase
{
  std::string name;
  // Set types follow the step add/remove approach:
  // 1 = block, 2 = nodeset, 3 = sideset.

  std::map<int, std::set<int>> added_preserved_sets;
  std::map<int, std::set<int>> removed_preserved_sets;

  // Global volume element ID -> preserved.
  std::map<int, bool> element_preserved;

  double target_volume_fraction;
  double void_stiffness_ratio;
  int maximum_iterations;

  std::map<int, int> iteration_jobs;
  int selected_iteration = -1;
};

struct TopologyOptions
{
  bool has_name = false;
  std::string name;

  bool has_volume_fraction = false;
  double volume_fraction = 0.0;

  bool has_void_ratio = false;
  double void_ratio = 0.0;

  bool has_maximum_iterations = false;
  int maximum_iterations = 0;
};

#endif