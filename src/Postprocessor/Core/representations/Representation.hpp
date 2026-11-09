#ifndef POSTPROCESSOR_REPRESENTATION_HPP
#define POSTPROCESSOR_REPRESENTATION_HPP

#include "ResultColoring.hpp"
#include "AnimationDefinition.hpp"
#include "ResultDeformation.hpp"
#include <string>
#include <cstddef>
#include <set>

namespace Postprocessor
{
  enum class OutputOwner
  {
    Source,
    Feature
  };
  enum class DatasetVisibilityAction
  {
    Show,
    Hide,
    ShowOnly,
    ShowAll
  };

  struct OutputReference
  {
    OutputOwner owner = OutputOwner::Source;
    std::string object_id;

    // Logical provider/feature output port, not a VTPC partition or block index.
    unsigned int port = 0;
  };

  // One representation connects one output to one view. Multiple representations
  // can show the same output in different views without duplicating source data.
  // Input and view references must resolve before the definition is accepted.
  // Feature outputs remain reserved until feature definitions are implemented.
  struct Representation
  {
    std::string id;
    std::string name;
    OutputReference input;
    std::string view_id;
    bool visible = true;
    // Per-view preferences, retained across frames. Empty means all datasets.
    // Source refresh clears these IDs because a new snapshot may reuse them.
    std::set<std::size_t> hidden_datasets;
    // Per-view frame selection; independent views may choose different frames.
    std::size_t increment_index = 0;
    ResultColoring coloring;
    ResultDeformation deformation;
    AnimationDefinition animation;
  };
  inline bool dataset_visible(const Representation& display, std::size_t dataset)
  {
    return display.visible && display.hidden_datasets.count(dataset) == 0;
  }
}

#endif // POSTPROCESSOR_REPRESENTATION_HPP
