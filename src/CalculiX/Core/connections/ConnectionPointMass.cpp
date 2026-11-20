#include "ConnectionPointMass.hpp"
#include <cmath>

bool ConnectionPointMass::validate(std::string& error) const
{
  if (id <= 0 || name.empty() || history_step < -1)
  {
    error = "A point mass requires a positive ID, a name and a valid history assignment.";
    return false;
  }
  if (!(mass_si > 0.0) || !std::isfinite(mass_si))
  {
    error = "Mass must be a positive finite value.";
    return false;
  }
  return true;
}
