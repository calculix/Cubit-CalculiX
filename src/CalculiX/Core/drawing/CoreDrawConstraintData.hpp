#ifndef COREDRAWCONSTRAINTDATA_HPP
#define COREDRAWCONSTRAINTDATA_HPP

#include <vector>

class CoreConstraints;

class CoreDrawConstraintData
{
public:
  static std::vector<std::vector<double>> get_draw_data_for_equation(CoreConstraints& constraints, int id);
  static std::vector<std::vector<double>> get_draw_data_for_equationgroup(CoreConstraints& constraints, int id);
};

#endif
