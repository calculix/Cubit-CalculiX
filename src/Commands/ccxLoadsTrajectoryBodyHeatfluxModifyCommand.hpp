#ifndef CCXLOADSTRAJECTORYBODYHEATFLUXMODIFYCOMMAND_HPP
#define CCXLOADSTRAJECTORYBODYHEATFLUXMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory modify command lets you modify your desired Trajectory Loads
 */
class ccxLoadsTrajectoryBodyHeatfluxModifyCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryBodyHeatfluxModifyCommand();
  ~ccxLoadsTrajectoryBodyHeatfluxModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYBODYHEATFLUXMODIFYCOMMAND_HPP