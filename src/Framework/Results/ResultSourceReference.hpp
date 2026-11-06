#ifndef CUBITSIM_RESULTSOURCEREFERENCE_HPP
#define CUBITSIM_RESULTSOURCEREFERENCE_HPP

#include <string>
#include <variant>
#include <vector>

namespace SimResults
{
  struct JobSourceReference
  {
    std::string application_id;
    std::string project_id;
    int job_id = -1;

    // Empty on initial opening means the current available run.
    // Published references identify the run actually loaded; a later job
    // execution must not silently replace an already published snapshot.
    std::string run_identity;
  };

  struct FileSourceReference
  {
    // Registered provider prefix, for example "ccx" or "vtk".
    std::string provider_id;

    // Nonempty list; companion FRD/DAT files may form one source.
    // Published paths are resolved absolute paths. VTPC child-file dependencies
    // remain the reader's responsibility, not separate Postprocessor sources.
    std::vector<std::string> paths;
  };

  using ResultSourceReference = std::variant<
    JobSourceReference,
    FileSourceReference>;
}

#endif // CUBITSIM_RESULTSOURCEREFERENCE_HPP
