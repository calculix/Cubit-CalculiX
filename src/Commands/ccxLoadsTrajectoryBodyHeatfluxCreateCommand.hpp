#ifndef CCXLOADSTRAJECTORYBODYHEATFLUXCREATECOMMAND_HPP
#define CCXLOADSTRAJECTORYBODYHEATFLUXCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory create command lets you create your desired Trajectory Loads
 */
class ccxLoadsTrajectoryBodyHeatfluxCreateCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryBodyHeatfluxCreateCommand();
  ~ccxLoadsTrajectoryBodyHeatfluxCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYBODYHEATFLUXCREATECOMMAND_HPP