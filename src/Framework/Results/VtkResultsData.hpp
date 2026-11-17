#ifndef CUBITSIM_VTKRESULTSDATA_HPP
#define CUBITSIM_VTKRESULTSDATA_HPP

#include <memory>
#include <array>
#include <optional>
#include <string>
#include <vector>
#include <cstddef>
#include "Framework/Core/units/SourceUnitContext.hpp"
#include "Framework/Core/units/QuantityUnit.hpp"
#include "ResultsTableMetadata.hpp"

class vtkDataObject;
class vtkTable;

namespace SimResults
{
  enum class LinkSource
  {
    CurrentModel,
    JobInput,
    FrdOnly
  };

  enum class DatasetLayout
  {
    SingleGrid,
    Collection
  };

  enum class ConversionStatus
  {
    NotConverted,
    Planning,
    Planned,
    Converting,
    Complete,
    Failed,
    Cancelled
  };

  enum class Association
  {
    Node,
    Element,
    IntegrationPoint,
    Table
  };

  enum class FieldRole
  {
    Unspecified,
    Displacement
  };

  struct Field
  {
    std::string name;
    std::string source;
    std::string set_name;
    Association association = Association::Node;
    std::vector<std::string> components;
    std::size_t dataset_id = 0;
    std::string array_name;
    std::string validity_array;
    bool interpolated = false;
    // Explicit physical vector components; derived extra columns are excluded.
    // Absent for scalars/tensors or when the provider cannot establish semantics.
    std::optional<std::array<std::size_t, 3>> vector_components;
    // Provider-established physical meaning. Vector shape or array names alone
    // do not establish displacement semantics for automatic deformation.
    FieldRole role = FieldRole::Unspecified;
    std::vector<SimUnits::QuantityUnit> component_units;
    bool categorical = false;
  };

  struct Dataset
  {
    std::size_t id = 0; // Collection item index; zero for SingleGrid.
    std::string name;
    std::string kind;
    int model_id = -1;
    std::size_t point_count = 0;
    std::size_t cell_count = 0;
    std::string element_type; // Empty when unknown; Mixed for heterogeneous sets.
    std::string material_id_array;
    std::string material_workspace;
    std::string material_project;
    // Native block RGB in [0,1], frozen with the result source, independent of fields.
    std::optional<std::array<double,3>> solid_color;
  };

  struct Increment
  {
    int step = -1;
    int increment = -1;
    int total_increment = -1;
    double time = 0.0;
    SimUnits::QuantityUnit independent_variable;
    std::string coordinate_unit;
    int analysis_type = -1;
    int mode = -1;
    int nodal_diameter = -1;
    int subcase = -1;
    std::vector<std::string> source_parameters;
    std::vector<Field> fields;
    // The builder must supply owning handles whose deleters release a VTK
    // reference (Delete()). Never wrap a borrowed VTK pointer with default delete.
    // One grid/collection per increment; unchanged arrays may be shared immutably.
    // VTK input APIs require non-const pointers. Consumers must treat published
    // buffers as immutable and use separate filter outputs for deformation.
    std::shared_ptr<vtkDataObject> dataset;
    std::vector<std::shared_ptr<vtkTable>> tables;
    std::vector<ResultsTableMetadata> table_metadata;
  };

  /** Complete, solver-independent result. No reader/model pointers are retained.
   * Publish as shared_ptr<const VtkResultsData> only after all tasks have joined
   * and every requested increment has been validated. All increments stay resident.
   * Forward declarations keep this ownership contract independent of VTK headers;
   * the dataset builder/exporter will supply the VTK dependency in later stages.
   */
  struct VtkResultsData
  {
    int job_id = -1;
    std::string run_identity;
    std::string result_identity;
    std::string solver_launch_id;
    SimUnits::SourceUnitContext source_units;
    std::string source_workspace;
    std::string source_project;
    std::string input_path;
    std::vector<std::string> input_files;
    LinkSource link_source = LinkSource::FrdOnly;
    DatasetLayout layout = DatasetLayout::SingleGrid;
    std::vector<std::string> diagnostics;
    std::vector<Increment> increments;
    std::vector<Dataset> datasets;
    std::vector<std::shared_ptr<vtkTable>> tables;
    std::vector<ResultsTableMetadata> table_metadata;
  };
}

#endif // CUBITSIM_VTKRESULTSDATA_HPP
