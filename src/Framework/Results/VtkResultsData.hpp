#ifndef FEA_VTKRESULTSDATA_HPP
#define FEA_VTKRESULTSDATA_HPP

#include <memory>
#include <string>
#include <vector>
#include <cstddef>

class vtkDataObject;
class vtkTable;

namespace FeaResults
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
  };

  struct Dataset
  {
    std::size_t id = 0; // Collection item index; zero for SingleGrid.
    std::string name;
    std::string kind;
    int model_id = -1;
    std::size_t point_count = 0;
    std::size_t cell_count = 0;
  };

  struct Increment
  {
    int step = -1;
    int increment = -1;
    int total_increment = -1;
    double time = 0.0;
    std::vector<Field> fields;
    // The builder must supply owning handles whose deleters release a VTK
    // reference (Delete()). Never wrap a borrowed VTK pointer with default delete.
    // One grid/collection per increment; unchanged arrays may be shared immutably.
    // VTK input APIs require non-const pointers. Consumers must treat published
    // buffers as immutable and use separate filter outputs for deformation.
    std::shared_ptr<vtkDataObject> dataset;
    std::vector<std::shared_ptr<vtkTable>> tables;
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
    std::string input_path;
    std::vector<std::string> input_files;
    LinkSource link_source = LinkSource::FrdOnly;
    DatasetLayout layout = DatasetLayout::SingleGrid;
    std::vector<std::string> diagnostics;
    std::vector<Increment> increments;
    std::vector<Dataset> datasets;
    std::vector<std::shared_ptr<vtkTable>> tables;
  };
}

#endif // FEA_VTKRESULTSDATA_HPP
