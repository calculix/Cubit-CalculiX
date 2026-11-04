#ifndef FEA_USEROPTIONSINTERFACE_HPP
#define FEA_USEROPTIONSINTERFACE_HPP

#include "UserOption.hpp"

class UserOptionsInterface
{
public:
  virtual ~UserOptionsInterface() = default;

  virtual const std::string& component_directory() const noexcept = 0;
  virtual const std::string& options_filename() const noexcept = 0;

  // Definitions and validators are borrowed only for the duration of each call.
  // load() materializes missing/invalid values from valid declared defaults.
  // save() requires exactly the declared keys; failure must not replace the file.
  // These operations serialize updates across cooperating Cubit processes.
  virtual bool load(const std::string& workspace, const std::vector<UserOption>& definitions,
    UserOptionValues& values, std::string& error) = 0;
  virtual bool save(const std::string& workspace, const std::vector<UserOption>& definitions,
    const UserOptionValues& values, std::string& error) = 0;
};

#endif
