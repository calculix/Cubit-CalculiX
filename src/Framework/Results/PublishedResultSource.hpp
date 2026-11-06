#ifndef CUBITSIM_PUBLISHEDRESULTSOURCE_HPP
#define CUBITSIM_PUBLISHEDRESULTSOURCE_HPP

#include "ResultSourceReference.hpp"
#include "VtkResultsData.hpp"

#include <memory>
#include <string>

namespace SimResults
{
  struct PublishedResultSource
  {
    // Resolved provenance of the published snapshot.
    ResultSourceReference reference;

    // Job name or filename stem. The Postprocessor applies any user-supplied
    // name override and checks session-wide uniqueness before committing it.
    std::string suggested_name;

    // Fully owned, validated data, independent of reader/provider lifetime.
    // Published VTK buffers are immutable by contract; consumers must use
    // separate filter outputs for display deformation and other processing.
    std::shared_ptr<const VtkResultsData> data;
  };
}

#endif // CUBITSIM_PUBLISHEDRESULTSOURCE_HPP
