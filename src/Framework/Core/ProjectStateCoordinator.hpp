#ifndef PROJECTSTATECOORDINATOR_HPP
#define PROJECTSTATECOORDINATOR_HPP
#include "CubitInterface.hpp"
#include "Framework/Interfaces/ProjectStateProviderInterface.hpp"
#include <vector>
class ProjectStateCoordinator : public CubitInterface::ObserverBase
{
public:
  explicit ProjectStateCoordinator(std::vector<ProjectStateProviderInterface*> providers);
  ~ProjectStateCoordinator() override;
  void notify_cub_file_read(std::string filename) override;
  void notify_cub_file_saved(std::string filename, bool temporary) override;
  void notify_model_reset() override;
private:
  void discard() noexcept;
  std::vector<ProjectStateProviderInterface*> providers;
  std::uint64_t generation = 0;
  bool updating = false;
};
#endif
