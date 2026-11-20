#include "ConnectionSpring.hpp"
#include <cmath>

bool ConnectionSpring::validate(std::string& error) const
{
  if (id <= 0 || name.empty() || history_step < -1)
  {
    error = "A spring requires a positive ID, a name and a valid history assignment.";
    return false;
  }
  if (!(stiffness_si > 0.0) || !std::isfinite(stiffness_si))
  {
    error = "Spring stiffness must be a positive finite value.";
    return false;
  }
  if (!second && (direction < 1 || direction > 3))
  {
    error = "A ground spring requires a global X, Y or Z direction.";
    return false;
  }
  if (second && (direction != 0 || (first.kind == second->kind && first.id == second->id)))
  {
    error = "An axial spring requires two distinct endpoints and uses the direction between them.";
    return false;
  }
  return true;
}

bool ConnectionSpring::uses_point(int point) const
{
  return first.uses_point(point) || (second && second->uses_point(point));
}
