#ifndef RESULTSOURCEPROVIDERINTERFACE_HPP
#define RESULTSOURCEPROVIDERINTERFACE_HPP

#include "Framework/Results/PublishedResultSource.hpp"

#include <string>

class ResultSourceProviderInterface
{
public:
  virtual ~ResultSourceProviderInterface() = default;

  // Stable registered prefix, independent of translated GUI workspace labels.
  // Framework registration maps application identity to this provider once.
  virtual std::string command_prefix() const = 0;

  // Called synchronously on the Cubit host thread. Internal worker tasks must
  // finish before returning. Does not activate workspaces, mutate the
  // Postprocessor session, or create viewport objects.
  // Success: validated non-null data, resolved provenance and an empty error.
  // Failure: output remains unchanged and error describes the failure.
  // Data and diagnostics are published together, never as a partial source.
  virtual bool open(
    const SimResults::ResultSourceReference& request,
    SimResults::PublishedResultSource& output,
    std::string& error) = 0;

  // Explicitly reloads the referenced job/files. A job refresh may resolve a
  // newer run, recorded in the output reference. The same publication and
  // failure guarantees as open() apply, preserving the old snapshot on failure.
  virtual bool refresh(
    const SimResults::ResultSourceReference& current,
    SimResults::PublishedResultSource& output,
    std::string& error) = 0;
};

#endif // RESULTSOURCEPROVIDERINTERFACE_HPP
