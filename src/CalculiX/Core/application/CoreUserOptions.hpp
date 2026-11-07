#ifndef COREUSEROPTIONS_HPP
#define COREUSEROPTIONS_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include "UserOptions.hpp"
#include <string>
#include <vector>

class UserOptionsInterface;

namespace CoreUserOptions
{
std::vector<UserOption> user_option_definitions();
bool values(UserOptionValues& snapshot, std::string& error);
bool initialize(UserOptionsInterface& store, std::string& error);
void detach() noexcept;
// Saved configuration for editing; ccx_uo holds the active session values.
UserOptions current();
std::vector<std::string> restart_required();
std::string component_directory();
bool apply(const UserOptionValues& changes, std::string& error);
}

#endif
