#ifndef CORETOPOLOGYMODELVALIDATION_HPP
#define CORETOPOLOGYMODELVALIDATION_HPP

#include <functional>
#include <set>

class CoreTopology;
class CoreSteps;
class CoreHBCs;
class CoreBCsDisplacements;
class MeshExportInterface;
class CoreSections;
class CoreBlocks;

class CoreContactPairs;
class CoreInitialConditions;

struct CoreTopologyModelValidationData
{
  CoreSteps& steps;
  CoreSections& sections;
  MeshExportInterface& mesh;
  CoreBCsDisplacements& displacements;
  const CoreContactPairs& contacts;
  const CoreInitialConditions& initialconditions;
  const CoreHBCs& hbcs;
  CoreBlocks* blocks;
};

class CoreTopologyModelValidation
{
public:
  /// Compile-time developer capability; never persisted in a topology case.
  static bool level_set_run_enabled();
  static bool validate_displacement_selection(const CoreTopology& topology, int topology_id, int iteration_id);
  static bool validate_run(const CoreTopology& topology, MeshExportInterface& mesh, int topology_id, const std::function<bool()>& validate_model);
  static bool validate_model(const CoreTopologyModelValidationData& data, bool require_zero_displacements);
  static bool validate_sections(CoreSections& sections, CoreBlocks& blocks);
  static bool validate_steps(CoreSteps& steps, const CoreHBCs& hbcs, std::set<int>& displacement_ids);
  static bool validate_displacements(CoreBCsDisplacements& displacements, MeshExportInterface& mesh, const std::set<int>& displacement_ids, bool require_zero_displacements);
};

#endif
