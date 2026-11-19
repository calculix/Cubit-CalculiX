#include "SourceProjectRuntime.hpp"
#include "../application/PostprocessorCore.hpp"

namespace Postprocessor
{
  const SourceRuntime* SourceProjectRuntime::find_source(const std::string& id) const noexcept
  {
    const auto found = loadedSources.find(id);
    if (found != loadedSources.end())
    {
      return &found->second;
    }
    return nullptr;
  }

  const SourceProjectRuntime::Sources& SourceProjectRuntime::sources() const noexcept
  {
    return loadedSources;
  }

  const SourceProjectRuntime::Bindings& SourceProjectRuntime::bindings() const noexcept
  {
    return savedBindings;
  }

  const SourceProjectRuntime::TableFrames& SourceProjectRuntime::table_frames() const noexcept
  {
    return savedTableFrames;
  }

  void SourceProjectRuntime::replace_sources(Sources& staged) noexcept
  {
    loadedSources.swap(staged);
  }

  void SourceProjectRuntime::replace_bindings(Bindings& staged) noexcept
  {
    savedBindings.swap(staged);
  }

  void SourceProjectRuntime::restore_project(PreparedPostprocessorProject& staged) noexcept
  {
    replace_sources(staged.runtime);
    replace_bindings(staged.definitions.bindings);
    savedTableFrames.swap(staged.definitions.table_frames);
  }

  void SourceProjectRuntime::clear_project() noexcept
  {
    loadedSources.clear();
    savedBindings.clear();
    savedTableFrames.clear();
  }

  void SourceProjectRuntime::invalidate_color_ranges() noexcept
  {
    for (auto& entry : loadedSources)
    {
      entry.second.all_frame_ranges.clear();
    }
  }

  const SourceRuntime* PostprocessorCore::source_runtime(const std::string& id) const noexcept
  {
    return sourceProjectRuntime->find_source(id);
  }

  std::optional<std::size_t> PostprocessorCore::source_first_mesh_frame(const std::string& id) const noexcept
  {
    if (const auto* loaded = source_runtime(id))
    {
      return loaded->first_mesh_increment;
    }
    return std::nullopt;
  }

  const std::vector<SimResults::Dataset>& PostprocessorCore::source_dataset_catalogue(const std::string& id) const noexcept
  {
    if (const auto* loaded = source_runtime(id))
    {
      return loaded->datasets;
    }
    static const std::vector<SimResults::Dataset> empty;
    return empty;
  }

  std::vector<SimResults::Dataset> PostprocessorCore::mesh_datasets(const std::string& id) const
  {
    return source_dataset_catalogue(id);
  }
}
