#include "CoreJointExport.hpp"
#include "CoreConnections.hpp"
#include <cmath>

bool CoreJointExport::append(const ConnectionJoint& joint, const std::array<int, 2>& side_a,
  const std::array<int, 2>& side_b, const std::array<double, 3>& axis, int guide,
  std::set<std::pair<int,int>>& dependent, std::ostream& output, std::string& error)
{
  int pivot = 0;
  for (int i = 1; i < 3; ++i)
  {
    if (std::abs(axis[i]) > std::abs(axis[pivot]))
    {
      pivot = i;
    }
  }
  for (int role = 0; role < 2; ++role)
  {
    if (role == 0 && guide)
    {
      // STRAIGHT selects transverse dependent components as the guide rotates.
      for (int dof = 1; dof <= 3; ++dof)
      {
        if (!dependent.emplace(side_a[0], dof).second)
        {
          error = "Slider reference node already constrained: " + joint.name;
          return false;
        }
      }
      output << "*MPC\nSTRAIGHT, " << side_b[0] << ", " << guide << ", " << side_a[0] << "\n";
      continue;
    }
    if (role == 1 && joint.type == "spherical")
    {
      continue;
    }
    const bool slide = role == 0 && (joint.type == "prismatic" || joint.type == "cylindrical");
    const bool spin = role == 1 && (joint.type == "revolute" || joint.type == "cylindrical");
    for (int i = 0; i < 3; ++i)
    {
      if ((slide || spin) && i == pivot)
      {
        continue;
      }
      const int a = side_a[role];
      const int b = side_b[role];
      const double ratio = (slide || spin) ? axis[i] / axis[pivot] : 0;
      if (!dependent.emplace(a, i + 1).second)
      {
        error = "Joint repeats a dependent degree of freedom: " + joint.name;
        return false;
      }
      if (b == 0 && ratio == 0)
      {
        output << "*BOUNDARY\n" << a << ", " << i+1 << ", " << i+1 << ", 0\n";
        continue;
      }
      const int terms = (b == 0 ? 1 : 2) * (ratio == 0 ? 1 : 2);
      output << "*EQUATION\n" << terms << "\n" << a << ", " << i+1 << ", 1";
      if (b > 0)
      {
        output << ", " << b << ", " << i+1 << ", -1";
      }
      if (ratio != 0)
      {
        output << ", " << a << ", " << pivot+1 << ", " << -ratio;
        if (b > 0)
        {
          output << ", " << b << ", " << pivot+1 << ", " << ratio;
        }
      }
      output << "\n";
    }
  }
  return true;
}
