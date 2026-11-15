#ifndef POSTPROCESSOR_SOURCERUNTIME_HPP
#define POSTPROCESSOR_SOURCERUNTIME_HPP

#include "Framework/Results/VtkResultsData.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include <utility>

#include <cstdint>
#include <memory>
#include <optional>
#include <map>

class vtkDataSet;
class vtkStaticCellLinks;

namespace Postprocessor
{
  enum class SourceAvailability
  {
    Unresolved, Available, Unavailable
  };
  // Transient data, excluded from persistent session definitions.
  struct SourceRuntime
  {
    SourceRuntime() = default;
    SourceRuntime(std::shared_ptr<const SimResults::VtkResultsData> snapshot, std::uint64_t generation,
      std::optional<std::size_t> first)
      : data(std::move(snapshot)), revision(generation), first_mesh_increment(first) {
      if (data) {
        availability = SourceAvailability::Available;
        groups = result_field_groups(*data, std::nullopt);
        datasets = result_dataset_catalogue(*data);
      }
    }
    std::shared_ptr<const SimResults::VtkResultsData> data;
    SourceAvailability availability = SourceAvailability::Unresolved;
    std::string unavailable_reason;

    // Starts at one and advances only on a successful refresh. Future consumers
    // must invalidate derived outputs when their source revision changes.
    std::uint64_t revision = 1;
    std::optional<std::size_t> first_mesh_increment;
    std::vector<ResultFieldGroup> groups;
    std::vector<SimResults::Dataset> datasets;
    // Per-field/per-dataset extrema, valid for this immutable snapshot. Visible
    // combinations are merged on demand and never create new cache entries.
    mutable std::map<std::string,std::optional<std::array<double,2>>> all_frame_ranges;
    mutable std::uint64_t unit_generation = 0;
    // Owned adjacency indexes; published meshes are never modified by BuildLinks.
    mutable std::map<vtkDataSet*,std::shared_ptr<vtkStaticCellLinks>> node_links;
  };
}

#endif // POSTPROCESSOR_SOURCERUNTIME_HPP
