#pragma once
#include "CoreTopologyConsole.hpp"
#include "CubitMessage.hpp"

#define TOPOLOGY_INFO(...) do { \
  if (CoreTopologyConsole::active()) { CoreTopologyConsole::write("", __VA_ARGS__); } \
  else { PRINT_INFO(__VA_ARGS__); } \
} while (false)
#define TOPOLOGY_WARNING(...) do { \
  if (CoreTopologyConsole::active()) { CoreTopologyConsole::write("Warning: ", __VA_ARGS__); } \
  else { PRINT_WARNING(__VA_ARGS__); } \
} while (false)
#define TOPOLOGY_ERROR(...) do { \
  if (CoreTopologyConsole::active()) { CoreTopologyConsole::write("Error: ", __VA_ARGS__); } \
  else { PRINT_ERROR(__VA_ARGS__); } \
} while (false)
