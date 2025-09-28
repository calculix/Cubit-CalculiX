#ifndef CCXLOADSTRAJECTORYBODYHEATFLUXSPHERECREATECOMMAND_HPP
#define CCXLOADSTRAJECTORYBODYHEATFLUXSPHERECREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory create command lets you create your desired Trajectory Loads
 */
class ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand();
  ~ccxLoadsTrajectoryBodyHeatfluxSphereCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYBODYHEATFLUXSPHERECREATECOMMAND_HPP