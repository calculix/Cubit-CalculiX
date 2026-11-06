#ifndef POSTPROCESSOR_SOURCEREFERENCE_HPP
#define POSTPROCESSOR_SOURCEREFERENCE_HPP

#include "Framework/Results/ResultSourceReference.hpp"

#include <string>
#include <variant>
#include <vector>

namespace Postprocessor
{
  using JobSourceReference = SimResults::JobSourceReference;
  using FileSourceReference = SimResults::FileSourceReference;

  struct CubitGeometryReference
  {
    // Resolved model identity and revision of the published geometry snapshot.
    // These are provider identities, not display names or live Cubit pointers.
    std::string model_identity;
    std::string model_revision;

    // Both lists empty means the complete model. Otherwise their union is used.
    // Volume IDs include their boundary geometry; surface IDs add explicit faces.
    std::vector<int> volume_ids;
    std::vector<int> surface_ids;
  };

  using SourceReference = std::variant<
    JobSourceReference,
    FileSourceReference,
    CubitGeometryReference>;
}

#endif // POSTPROCESSOR_SOURCEREFERENCE_HPP
