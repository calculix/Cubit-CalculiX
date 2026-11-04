#ifndef COREUSEROPTIONS_HPP
#define COREUSEROPTIONS_HPP

#include "UserOptions.hpp"
#include <string>
#include <vector>

class UserOptionsInterface;

namespace CoreUserOptions
{
bool initialize(UserOptionsInterface& store, std::string& error);
void detach() noexcept;
// Saved configuration for editing; ccx_uo holds the active session values.
UserOptions current();
std::vector<std::string> restart_required();
std::string component_directory();
bool apply(const std::string& payload, std::string& error);
}

#endif
