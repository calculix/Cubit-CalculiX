#ifndef CCXLOADSTRAJECTORYBODYHEATFLUXSPHEREMODIFYCOMMAND_HPP
#define CCXLOADSTRAJECTORYBODYHEATFLUXSPHEREMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory modify command lets you modify your desired Trajectory Loads
 */
class ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand();
  ~ccxLoadsTrajectoryBodyHeatfluxSphereModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYBODYHEATFLUXSPHEREMODIFYCOMMAND_HPP