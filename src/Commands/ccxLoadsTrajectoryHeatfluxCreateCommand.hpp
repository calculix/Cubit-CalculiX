#ifndef CCXLOADSTRAJECTORYHEATFLUXCREATECOMMAND_HPP
#define CCXLOADSTRAJECTORYHEATFLUXCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory create command lets you create your desired Trajectory Loads
 */
class ccxLoadsTrajectoryHeatfluxCreateCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryHeatfluxCreateCommand();
  ~ccxLoadsTrajectoryHeatfluxCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYHEATFLUXCREATECOMMAND_HPP