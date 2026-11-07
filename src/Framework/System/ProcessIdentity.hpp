#ifndef CUBITSIM_PROCESSIDENTITY_HPP
#define CUBITSIM_PROCESSIDENTITY_HPP

#include <cstdint>

struct ProcessIdentity
{
  std::uint64_t pid = 0;
  // Native creation FILETIME on Windows; /proc starttime ticks on Linux.
  // Runtime-only, meaningful on this machine during the current boot.
  std::uint64_t birthToken = 0;
};

#endif
