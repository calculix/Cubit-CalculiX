#ifndef CUBITSIM_USEROPTIONSSTORE_HPP
#define CUBITSIM_USEROPTIONSSTORE_HPP

#include "StorageExport.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include "Framework/Utility/FrameworkPaths.hpp"

class CUBITSIM_STORAGE_EXPORT UserOptionsStore : public UserOptionsInterface
{
public:
  explicit UserOptionsStore(FrameworkPaths paths);

  const std::string& component_directory() const noexcept override;
  const std::string& options_filename() const noexcept override;
  bool load(const std::string& workspace, const std::vector<UserOption>& definitions,
    UserOptionValues& values, std::string& error) override;
  bool save(const std::string& workspace, const std::vector<UserOption>& definitions,
    const UserOptionValues& values, std::string& error) override;

private:
  bool transact(const std::string& workspace, const std::vector<UserOption>& definitions,
    UserOptionValues& values, bool loading, std::string& error);
  FrameworkPaths paths;
};

#endif
