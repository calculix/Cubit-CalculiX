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

namespace Postprocessor
{
  // Transient data, excluded from persistent session definitions.
  struct SourceRuntime
  {
    SourceRuntime() = default;
    SourceRuntime(std::shared_ptr<const SimResults::VtkResultsData> snapshot, std::uint64_t generation,
      std::optional<std::size_t> first)
      : data(std::move(snapshot)), revision(generation), first_mesh_increment(first),
        groups(data ? result_field_groups(*data,std::nullopt) : std::vector<ResultFieldGroup>{}),
        datasets(data ? result_dataset_catalogue(*data) : std::vector<SimResults::Dataset>{})
    {
    }
    std::shared_ptr<const SimResults::VtkResultsData> data;

    // Starts at one and advances only on a successful refresh. Future consumers
    // must invalidate derived outputs when their source revision changes.
    std::uint64_t revision = 1;
    std::optional<std::size_t> first_mesh_increment;
    std::vector<ResultFieldGroup> groups;
    std::vector<SimResults::Dataset> datasets;
    // Per-field/per-dataset extrema, valid for this immutable snapshot. Visible
    // combinations are merged on demand and never create new cache entries.
    mutable std::map<std::string,std::optional<std::array<double,2>>> all_frame_ranges;
  };
}

#endif // POSTPROCESSOR_SOURCERUNTIME_HPP
