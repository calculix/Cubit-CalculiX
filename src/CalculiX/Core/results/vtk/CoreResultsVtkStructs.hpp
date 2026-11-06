#ifndef CORERESULTSVTKSTRUCTS_HPP
#define CORERESULTSVTKSTRUCTS_HPP

#include "Framework/Results/VtkResultsData.hpp"
#include "ElementIntegration.hpp"
#include <string>
#include <vector>

struct CoreResultsVtkOptions
{
  // Accept abs(a-b) <= absolute + relative*max(abs(a),abs(b)), per coordinate,
  // or an exact match after the candidate's float/%12.5E ASCII FRD conversion.
  double coordinate_absolute_tolerance = 1.0e-10;
  double coordinate_relative_tolerance = 2.0e-6;
  double time_absolute_tolerance = 1.0e-10;
  double time_relative_tolerance = 2.0e-6;
  bool include_frd = true;
  bool include_dat = true;
  bool include_partial = true;
  // Zero inherits the existing converter-thread setting in the results core.
  std::size_t workers = 0;
  std::size_t chunk_rows = 4096;
  std::size_t chunk_bytes = 2 * 1024 * 1024;
  std::size_t pending_bytes = 64 * 1024 * 1024;
  bool interpolate_ip_displacement = true;
  // Empty means all groups of the corresponding kind.
  std::vector<int> block_ids;
  std::vector<int> nodeset_ids;
  std::vector<int> sideset_ids;
};

struct CoreResultsVtkNode
{
  int id = -1;
  ElementIntegration::Coordinates coordinates{};
};

struct CoreResultsVtkElement
{
  int id = -1;
  int frd_type = 0;
  // Empty in an FRD-only mesh: FRD topology does not establish the formulation.
  std::string calculix_type;
  std::vector<int> connectivity; // Global node IDs, always CalculiX order.
};

enum class CoreResultsVtkGroupKind
{
  Block,
  Nodeset,
  Surface,
  DatNodes,
  DatElements,
  DatIntegrationPoints
};

struct CoreResultsVtkSide
{
  int element_id = -1;
  std::string side; // CCX label S1..S6, SPOS or SNEG; never a VTK face index.
};

struct CoreResultsVtkPointKey
{
  int entity_id = -1; // Node ID, or element ID when integration_point > 0.
  int integration_point = 0;

  bool operator<(const CoreResultsVtkPointKey& other) const
  {
    return entity_id < other.entity_id || (entity_id == other.entity_id && integration_point < other.integration_point);
  }

  bool operator==(const CoreResultsVtkPointKey& other) const
  {
    return entity_id == other.entity_id && integration_point == other.integration_point;
  }
};

struct CoreResultsVtkGroup
{
  int model_id = -1; // INP names need not encode a Cubit ID.
  std::string name;
  CoreResultsVtkGroupKind kind = CoreResultsVtkGroupKind::Block;
  std::vector<int> ids;
  std::vector<CoreResultsVtkSide> sides;
};

struct CoreResultsVtkMesh
{
  // Sorted by global ID, unique; connectivity is validated against these nodes.
  std::vector<CoreResultsVtkNode> nodes;
  std::vector<CoreResultsVtkElement> elements;
  std::vector<CoreResultsVtkGroup> groups;
};

struct CoreResultsVtkFieldPlan
{
  SimResults::Field field;
  int source_block = -1; // Index in the existing reader's result_blocks.
  int source_data = -1;
  std::size_t component_offset = 0; // DAT element/IP identifiers are not values.
};

struct CoreResultsVtkIncrementPlan
{
  int step = -1;
  int increment = -1;
  int total_increment = -1;
  double time = 0.0;
  int analysis_type = -1;
  int mode = -1;
  int nodal_diameter = -1;
  int subcase = -1;
  std::vector<std::string> source_parameters;
  std::vector<CoreResultsVtkFieldPlan> fields;
  // Per-record sources such as FRD CELS; repeated reported node IDs are valid.
  // Source indices refer to the same reader generation as spatial fields.
  std::vector<CoreResultsVtkFieldPlan> table_fields;
};

struct CoreResultsVtkTablePlan
{
  // References to the existing DAT reader, not a new DAT output parser.
  enum class Kind
  {
    Buckling,
    Section,
    DatResult
  };
  Kind kind = Kind::Buckling;
  std::size_t source_data = 0;
  int increment = -1; // -1 means a global table; otherwise a plan increment index.
};

/** Owned plan: no live Cubit handles or reader pointers. Source block indices
 * refer to the exact loaded reader generation used to prepare this plan; callers
 * must prevent reload/deletion while a later conversion consumes those indices.
 * A plan is not a converted dataset and must never be published as Complete.
 */
struct CoreResultsVtkPlan
{
  int job_id = -1;
  std::string run_identity;
  CoreResultsVtkOptions options;
  SimResults::LinkSource link_source = SimResults::LinkSource::FrdOnly;
  SimResults::DatasetLayout layout = SimResults::DatasetLayout::SingleGrid;
  std::string input_path;
  std::vector<std::string> input_files;
  std::vector<std::string> diagnostics;
  CoreResultsVtkMesh mesh;
  std::vector<CoreResultsVtkIncrementPlan> increments;
  std::vector<CoreResultsVtkTablePlan> tables;
};

#endif // CORERESULTSVTKSTRUCTS_HPP
