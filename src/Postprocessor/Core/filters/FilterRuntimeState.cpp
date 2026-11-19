#include "FilterRuntimeState.hpp"
#include "../application/PostprocessorCore.hpp"
#include "../session/PostprocessorSession.hpp"
#include <utility>

namespace Postprocessor
{
  std::shared_ptr<const EvaluatedOutput> FilterRuntimeState::find(const Representation& display,
    std::uint64_t revision,const std::shared_ptr<const SimResults::VtkResultsData>& source) const
  {
    const auto cached = outputs.find(display.id);
    if (cached != outputs.end() && cached->second.source == source && cached->second.revision == revision &&
        cached->second.frame == display.increment_index && same_deformation(cached->second.deformation,display.deformation))
    {
      return cached->second.output;
    }
    return {};
  }

  void FilterRuntimeState::store(const Representation& display,std::uint64_t revision,
    const std::shared_ptr<const SimResults::VtkResultsData>& source,std::shared_ptr<const EvaluatedOutput> output)
  {
    outputs[display.id] = {revision,display.increment_index,display.deformation,source,std::move(output)};
  }

  void FilterRuntimeState::prune(const PostprocessorSession& session)
  {
    for (auto item = outputs.begin(); item != outputs.end();)
    {
      const auto display = session.representations.find(item->first);
      if (display == session.representations.end() || !display->second.visible)
      {
        item = outputs.erase(item);
      }
      else
      {
        ++item;
      }
    }
  }

  void FilterRuntimeState::swap(FilterRuntimeState& staged) noexcept
  {
    outputs.swap(staged.outputs);
  }

  void FilterRuntimeState::clear() noexcept
  {
    outputs.clear();
  }

  void PostprocessorCore::invalidate_filter_runtime() noexcept
  {
    filterRuntime->clear();
  }
}
