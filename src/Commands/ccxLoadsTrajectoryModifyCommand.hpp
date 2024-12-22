#ifndef CCXLOADSTRAJECTORYMODIFYCOMMAND_HPP
#define CCXLOADSTRAJECTORYMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory modify command lets you modify your desired Trajectory Loads
 */
class ccxLoadsTrajectoryModifyCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryModifyCommand();
  ~ccxLoadsTrajectoryModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYMODIFYCOMMAND_HPP