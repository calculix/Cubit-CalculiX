#ifndef CALCULIXRESULTSOURCEPROVIDER_HPP
#define CALCULIXRESULTSOURCEPROVIDER_HPP

#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"

// Owned by CalculiXModule; used only after Core initialization and before module
// shutdown, on the Cubit host thread. No access to CalculiX GUI internals.
class CalculiXResultSourceProvider final : public ResultSourceProviderInterface
{
public:
  std::string command_prefix() const override;
  bool available_jobs(std::vector<SimResults::ResultJobEntry>& output, std::string& error) const override;
  bool restore(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override;

  bool open(
    const SimResults::ResultSourceReference& request,
    SimResults::PublishedResultSource& output,
    std::string& error) override;

  bool refresh(
    const SimResults::ResultSourceReference& current,
    SimResults::PublishedResultSource& output,
    std::string& error) override;

private:
  // Explicit snapshot requests are matched against retained converted data only.
  // Refresh deliberately reloads current files instead of matching the old token.
  // Published output is replaced only on success. Existing CalculiX reader/cache
  // mutations during loading are not transactional; caller-held snapshots survive.
  bool load_job(
    const SimResults::JobSourceReference& request,
    bool refresh,
    SimResults::PublishedResultSource& output,
    std::string& error);
};

#endif // CALCULIXRESULTSOURCEPROVIDER_HPP
