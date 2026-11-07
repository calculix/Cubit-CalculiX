#ifndef POSTPROCESSORUSEROPTIONS_HPP
#define POSTPROCESSORUSEROPTIONS_HPP

#include "Framework/Interfaces/UserOption.hpp"

class UserOptionsInterface;

namespace Postprocessor
{
struct UserOptions
{
  std::string iconsPath;
};

class PostprocessorUserOptions
{
public:
  static std::vector<UserOption> definitions();
  bool initialize(UserOptionsInterface& store, std::string& error);
  void detach() noexcept;
  const UserOptions& current() const noexcept;
  const std::string& active_icons_path() const noexcept;
  bool restart_required() const;
  bool values(UserOptionValues& snapshot, std::string& error) const;
  // Typed edits are validated and merged before saving and publication.
  // Failure leaves both saved configuration and active session values unchanged.
  bool apply(const UserOptionValues& changes, std::string& error);

private:
  UserOptionsInterface* store = nullptr;
  UserOptions configured;
  std::string activeIconsPath;
};
}

#endif
