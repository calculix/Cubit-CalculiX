#include "PostprocessorCore.hpp"
#include <set>

namespace Postprocessor
{
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
    const auto loaded = runtime.find(source_id);
    if (loaded == runtime.end())
    {
      return {};
    }
    auto groups = loaded->second.groups;
    std::set<std::string> available;
    if (loaded->second.data && current && *current < loaded->second.data->increments.size())
    {
      for (const auto& field : loaded->second.data->increments[*current].fields)
      {
        available.insert(field_group_key(field));
      }
    }
    for (auto& group : groups)
    {
      group.available = available.find(group.key) != available.end();
    }
    return groups;
  }

  std::shared_ptr<const SimResults::VtkResultsData> PostprocessorCore::result_data(const std::string& source_id) const
  {
    const auto entry = runtime.find(source_id);

    if (entry == runtime.end())
    {
      return {};
    }

    return entry->second.data;
  }
}
