#ifndef POSTPROCESSOR_RESULTPROBE_HPP
#define POSTPROCESSOR_RESULTPROBE_HPP
#include "EntityReference.hpp"
#include "../representations/Representation.hpp"

namespace Postprocessor
{
  struct PostprocessorSession;
  // Empty source_id clears every view. Overflow rejection occurs before mutation.
  bool clear_source_selection(PostprocessorSession& session, const std::string& source_id, std::string& error);
  struct ProbeResult
  {
    EntityReference entity;
    std::string source_name, dataset_name, status, field, component, value_status;
    bool available = false;
    bool visible = false;
    bool range_value = false;
    std::optional<std::array<double,3>> original_position, displayed_position;
    std::optional<std::array<double,2>> values;
    std::size_t valid_samples = 0, invalid_samples = 0;
    int step = -1, increment = -1;
    double time = 0;
  };
  // Integral ID arrays only: conversions through double would lose 64-bit IDs.
  bool entity_from_index(vtkDataSet* mesh, std::size_t index, EntityKind kind,
    const std::string& source_id, std::size_t dataset, EntityReference& entity, std::string& error);
  std::vector<std::size_t> entity_indices(vtkDataSet* mesh, const EntityReference& entity, std::string& error);
  // No averaging or extrapolation. Elements sample cell data directly or report
  // the min/max of valid values over their original nodes. Masks and declared
  // vector columns follow coloring semantics. Conflicting duplicate IDs report
  // ranges/multiple positions instead of silently selecting the first tuple.
  ProbeResult probe_result(const SimResults::VtkResultsData& data, const Representation& display,
    const EntityReference& entity);
  std::string probe_text(const ProbeResult& result);
}
#endif
