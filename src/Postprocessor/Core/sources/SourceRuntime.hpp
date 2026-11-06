#ifndef POSTPROCESSOR_SOURCERUNTIME_HPP
#define POSTPROCESSOR_SOURCERUNTIME_HPP

#include "Framework/Results/VtkResultsData.hpp"

#include <cstdint>
#include <memory>

namespace Postprocessor
{
  // Transient data, excluded from persistent session definitions.
  struct SourceRuntime
  {
    std::shared_ptr<const SimResults::VtkResultsData> data;

    // Starts at one and advances only on a successful refresh. Future consumers
    // must invalidate derived outputs when their source revision changes.
    std::uint64_t revision = 1;
  };
}

#endif // POSTPROCESSOR_SOURCERUNTIME_HPP
