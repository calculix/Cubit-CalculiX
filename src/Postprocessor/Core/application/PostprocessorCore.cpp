#include "PostprocessorCore.hpp"

namespace Postprocessor
{
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
