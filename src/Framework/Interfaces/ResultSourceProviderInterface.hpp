#ifndef RESULTSOURCEPROVIDERINTERFACE_HPP
#define RESULTSOURCEPROVIDERINTERFACE_HPP

#include "Framework/Results/PublishedResultSource.hpp"
#include "Framework/Results/ResultJobEntry.hpp"
#include <vector>

#include <string>

class ResultSourceProviderInterface
{
public:
  virtual ~ResultSourceProviderInterface() = default;
  // Host-thread metadata only: no result reads, conversion, or session mutation.
  virtual bool available_jobs(std::vector<SimResults::ResultJobEntry>& output, std::string& error) const
  {
    output.clear();
    error.clear();
    return true;
  }


  // Stable registered prefix, independent of translated GUI workspace labels.
  // Framework registration maps application identity to this provider once.
  virtual std::string command_prefix() const = 0;
  // Project restoration must not substitute the latest on-disk solver run.
  // Providers opt in explicitly; failure leaves the source unresolved.
  virtual bool restore(const SimResults::ResultSourceReference&, SimResults::PublishedResultSource&, std::string& error)
  {
    error = "This provider cannot restore saved result references.";
    return false;
  }

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
