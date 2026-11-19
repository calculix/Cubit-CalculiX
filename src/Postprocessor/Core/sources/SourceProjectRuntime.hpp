#pragma once
#include "../persistence/PostprocessorProjectState.hpp"

namespace Postprocessor
{
  // The live sources and restore bindings share a transaction boundary. Callers
  // stage changes in separate maps and publish with non-throwing swaps.
  class SourceProjectRuntime
  {
  public:
    using Sources = std::map<std::string,SourceRuntime>;
    using Bindings = std::map<std::string,SavedRepresentationBinding>;
    using TableFrames = std::map<std::string,SavedFrameReference>;

    const SourceRuntime* find_source(const std::string& id) const noexcept;
    const Sources& sources() const noexcept;
    const Bindings& bindings() const noexcept;
    const TableFrames& table_frames() const noexcept;
    void replace_sources(Sources& staged) noexcept;
    void replace_bindings(Bindings& staged) noexcept;
    void restore_project(PreparedPostprocessorProject& staged) noexcept;
    void clear_project() noexcept;
    void invalidate_color_ranges() noexcept;

  private:
    Sources loadedSources;
    Bindings savedBindings;
    TableFrames savedTableFrames;
  };
}
