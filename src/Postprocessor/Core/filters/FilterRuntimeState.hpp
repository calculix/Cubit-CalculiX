#pragma once
#include "FilterRuntime.hpp"

namespace Postprocessor
{
  struct PostprocessorSession;

  class FilterRuntimeState
  {
  public:
    std::shared_ptr<const EvaluatedOutput> find(const Representation& display,std::uint64_t revision,
      const std::shared_ptr<const SimResults::VtkResultsData>& source) const;
    void store(const Representation& display,std::uint64_t revision,
      const std::shared_ptr<const SimResults::VtkResultsData>& source,std::shared_ptr<const EvaluatedOutput> output);
    void prune(const PostprocessorSession& session);
    void swap(FilterRuntimeState& staged) noexcept;
    void clear() noexcept;

  private:
    struct CachedFilterOutput
    {
      std::uint64_t revision = 0;
      std::size_t frame = 0;
      ResultDeformation deformation;
      std::shared_ptr<const SimResults::VtkResultsData> source;
      std::shared_ptr<const EvaluatedOutput> output;
    };
    std::map<std::string,CachedFilterOutput> outputs;
  };
}
