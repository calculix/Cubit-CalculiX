#include "PostprocessorCore.hpp"
#include "../explosion/ExplosionRuntimeState.hpp"
#include "../filters/FilterRuntimeState.hpp"
#include "../representations/AnimationRotationRuntime.hpp"
#include "../sources/SourceProjectRuntime.hpp"
#include <set>

namespace Postprocessor
{
  PostprocessorCore::PostprocessorCore()
    : explosionRuntime(std::make_unique<ExplosionRuntimeState>()),
      filterRuntime(std::make_unique<FilterRuntimeState>()),
      animationRuntime(std::make_unique<AnimationRotationPlayback>()),
      sourceProjectRuntime(std::make_unique<SourceProjectRuntime>())
  {
  }

  PostprocessorCore::~PostprocessorCore() = default;

  void PostprocessorCore::set_default_palette(ColorPalette palette) noexcept
  {
    default_palette = palette;
  }

  const PostprocessorSession& PostprocessorCore::session() const noexcept
  {
    return state;
  }

  std::string PostprocessorCore::source_id_from_name(const std::string& name) const
  {
    for (const auto& entry : state.sources)
    {
      if (entry.second.name == name)
      {
        return entry.first;
      }
    }

    return {};
  }

  std::vector<ResultFieldGroup> PostprocessorCore::field_groups(const std::string& source_id, std::optional<std::size_t> current) const
  {
    const auto* loaded = source_runtime(source_id);
    if (!loaded)
    {
      return {};
    }
    auto groups = loaded->groups;
    std::set<std::string> available;
    if (loaded->data && current && *current < loaded->data->increments.size())
    {
      for (const auto& field : loaded->data->increments[*current].fields)
      {
        available.insert(field_group_key(field));
      }
    }
    for (auto& group : groups)
    {
      group.available = group.unit_issue.empty() && available.find(group.key) != available.end();
    }
    return groups;
  }

  std::shared_ptr<const SimResults::VtkResultsData> PostprocessorCore::result_data(const std::string& source_id) const
  {
    const auto* entry = source_runtime(source_id);

    if (!entry)
    {
      return {};
    }

    return entry->data;
  }
}
