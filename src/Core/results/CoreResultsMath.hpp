#ifndef CORERESULTSMATH_HPP
#define CORERESULTSMATH_HPP

#include <vector>

class CoreResultsMath
{
public:
  static double compute_von_mises_stress(const std::vector<double>& values);
  static double compute_von_mises_strain(const std::vector<double>& values);
  static std::vector<double> compute_principal_stresses(const std::vector<double>& values);
  static std::vector<double> compute_principal_strains(const std::vector<double>& values);
};

#endif
