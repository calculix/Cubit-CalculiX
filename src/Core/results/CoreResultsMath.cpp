#include "CoreResultsMath.hpp"

#include <cmath>
#include <Utility/Eigen/Eigenvalues>

namespace
{
  // Input order: XX, YY, ZZ, XY, YZ, ZX.
  // Output: descending principal values, followed by the signed largest magnitude.
  std::vector<double> compute_principal_values(const std::vector<double>& values)
  {
    Eigen::Matrix3d tensor;
    tensor << values[0], values[3], values[5],
              values[3], values[1], values[4],
              values[5], values[4], values[2];

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(tensor);

    std::vector<double> principal(3);
    for (int i = 0; i < 3; ++i)
    {
      principal[i] = solver.eigenvalues()[2 - i];
    }

    double worst = 0.0;
    for (double value : principal)
    {
      if (std::fabs(worst) < std::fabs(value))
      {
        worst = value;
      }
    }

    principal.push_back(worst);
    return principal;
  }
}

double CoreResultsMath::compute_von_mises_stress(const std::vector<double>& values)
{
  const double xy = values[0] - values[1];
  const double yz = values[1] - values[2];
  const double zx = values[2] - values[0];
  const double shear = values[3] * values[3] + values[4] * values[4] + values[5] * values[5];

  return std::sqrt(0.5 * (xy * xy + yz * yz + zx * zx) + 3.0 * shear);
}

double CoreResultsMath::compute_von_mises_strain(const std::vector<double>& values)
{
  return (2.0 / 3.0) * compute_von_mises_stress(values);
}

std::vector<double> CoreResultsMath::compute_principal_stresses(const std::vector<double>& values)
{
  return compute_principal_values(values);
}

std::vector<double> CoreResultsMath::compute_principal_strains(const std::vector<double>& values)
{
  return compute_principal_values(values);
}
