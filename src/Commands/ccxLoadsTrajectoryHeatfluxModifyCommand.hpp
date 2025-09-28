#ifndef CCXLOADSTRAJECTORYHEATFLUXMODIFYCOMMAND_HPP
#define CCXLOADSTRAJECTORYHEATFLUXMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory modify command lets you modify your desired Trajectory Loads
 */
class ccxLoadsTrajectoryHeatfluxModifyCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryHeatfluxModifyCommand();
  ~ccxLoadsTrajectoryHeatfluxModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYHEATFLUXMODIFYCOMMAND_HPP