#ifndef POSTPROCESSOR_REPRESENTATION_HPP
#define POSTPROCESSOR_REPRESENTATION_HPP

#include <string>
#include <cstddef>

namespace Postprocessor
{
  enum class OutputOwner
  {
    Source,
    Feature
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
    // Per-view frame selection; independent views may choose different frames.
    std::size_t increment_index = 0;
  };
}

#endif // POSTPROCESSOR_REPRESENTATION_HPP
