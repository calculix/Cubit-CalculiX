#ifndef POSTPROCESSOR_DATASOURCE_HPP
#define POSTPROCESSOR_DATASOURCE_HPP

#include "SourceReference.hpp"

#include <string>

namespace Postprocessor
{
  // Persistent source definition. ID is internal, stable and session-scoped.
  // Name is nonempty, unique within the session and matched exactly, including
  // case, by commands. Renaming preserves references, which use the internal ID.
  // Default names come from the job name or filename stem; an explicit name
  // overrides the default. Collisions require an explicit distinct name and must
  // never silently replace a source. Core operations enforce these contracts.
  // Loaded arrays, loading status, diagnostics and runtime revisions belong to
  // separate runtime storage keyed by source ID.
  struct DataSource
  {
    std::string id;
    std::string name;
    SourceReference reference;
  };
}

#endif // POSTPROCESSOR_DATASOURCE_HPP
