#ifndef CUBITSIM_RESULTSOURCEREFERENCE_HPP
#define CUBITSIM_RESULTSOURCEREFERENCE_HPP

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace SimResults
{
  inline constexpr double default_topology_threshold = 0.5;

  struct JobSourceReference
  {
    std::string application_id;
    std::string project_id;
    int job_id = -1;

    // Empty on initial opening means the current available run.
    // Published references identify the run actually loaded; a later job
    // execution must not silently replace an already published snapshot.
    std::string run_identity;
    std::string result_identity; // Durable content/link fingerprint; independent of conversion tokens.
    // When set, job_id addresses a topology case in the same provider.
    std::optional<double> topology_threshold;
    int topology_last_iteration = 0; // Saved snapshot boundary for restore.
  };

  struct FileSourceReference
  {
    // Registered provider prefix, for example "ccx" or "vtk".
    std::string provider_id;

    // Nonempty list; companion FRD/DAT files may form one source.
    // Published paths are resolved absolute paths. VTPC child-file dependencies
    // remain the reader's responsibility, not separate Postprocessor sources.
    std::vector<std::string> paths;
    std::string content_identity; // SHA-256 of the standalone file when supported.
  };

  using ResultSourceReference = std::variant<
    JobSourceReference,
    FileSourceReference>;
}

#endif // CUBITSIM_RESULTSOURCEREFERENCE_HPP
