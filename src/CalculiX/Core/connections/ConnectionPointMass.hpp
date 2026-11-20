#pragma once
#include "ConnectionEndpoint.hpp"

struct ConnectionPointMass
{
  int id = 0;
  std::string name;
  ConnectionEndpoint endpoint;
  double mass_si = 0.0;
  int history_step = -1;
  bool validate(std::string& error) const;
};
