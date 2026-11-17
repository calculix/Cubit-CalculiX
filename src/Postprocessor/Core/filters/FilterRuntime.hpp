#pragma once
#include "FilterDefinition.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <memory>
#include <map>
class vtkDataSet;
class vtkPolyData;
namespace Postprocessor
{
  std::string filter_coordinate_unit(const SimResults::VtkResultsData& source, std::size_t frame);
  struct FilteredPart
  {
    std::size_t dataset = 0;
    std::shared_ptr<vtkDataSet> mesh;
    std::shared_ptr<vtkDataSet> removed;
    std::shared_ptr<vtkPolyData> nodes, edges, reference_edges;
  };
  struct EvaluatedOutput
  {
    std::shared_ptr<const SimResults::VtkResultsData> source, data;
    std::string source_id;
    std::vector<FilteredPart> parts;
    std::map<std::size_t,std::string> warnings;
  };
  // Produces private displayed-coordinate geometry. Published source buffers are
  // immutable. Source element IDs are cell attributes; synthesized point IDs are
  // never solver identities. Nodal masks become NaN before interpolation.
  bool evaluate_filter(const FilterDefinition& filter,const Representation& display,
    std::shared_ptr<const SimResults::VtkResultsData> source,EvaluatedOutput& output,std::string& error);
  std::shared_ptr<vtkPolyData> filter_parent_edges(vtkDataSet* mesh,const FilterDefinition& filter,
    const SimResults::Increment& frame,std::size_t dataset,const std::vector<std::size_t>& cells,std::string& error);
  bool evaluate_clip(vtkDataSet* input,const FilterDefinition& filter,std::shared_ptr<vtkDataSet>& output,std::string& error);
  bool evaluate_section(vtkDataSet* input,const FilterDefinition& filter,std::shared_ptr<vtkDataSet>& output,std::string& error);
  struct CachedFilterOutput
  {
    std::uint64_t revision = 0;
    std::size_t frame = 0;
    ResultDeformation deformation;
    std::shared_ptr<const SimResults::VtkResultsData> source;
    std::shared_ptr<const EvaluatedOutput> output;
  };
}
