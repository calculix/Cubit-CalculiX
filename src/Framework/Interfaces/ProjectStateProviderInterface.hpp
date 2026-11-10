#ifndef PROJECTSTATEPROVIDERINTERFACE_HPP
#define PROJECTSTATEPROVIDERINTERFACE_HPP
#include <string>
#include <cstdint>
struct ProjectStateContext
{
  std::string filename;
  std::uint64_t generation = 0;
};
// Host-thread, ordered native project lifecycle. Each writer owns and closes its
// HDF5 handles. This guarantees ordering, not rollback across all workspaces.
class ProjectStateProviderInterface
{
public:
  virtual ~ProjectStateProviderInterface() = default;
  virtual std::string project_state_id() const = 0;
  virtual void set_project_coordinated(bool)
  {
  }
  virtual bool prepare_project_read(const ProjectStateContext&, std::string&)
  {
    return true;
  }
  virtual bool restore_project_model(const ProjectStateContext&, std::string&)
  {
    return true;
  }
  virtual bool resolve_project_sources(const ProjectStateContext&, std::string&)
  {
    return true;
  }
  virtual bool commit_project_read(const ProjectStateContext&, std::string&)
  {
    return true;
  }
  virtual void discard_project_read() noexcept
  {
  }
  virtual bool prepare_project_save(const ProjectStateContext&, std::string&)
  {
    return true;
  }
  virtual bool write_project_state(const ProjectStateContext&, std::string&) = 0;
};
#endif
