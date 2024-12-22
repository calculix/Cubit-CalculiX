#ifndef CCXLOADSTRAJECTORYCREATECOMMAND_HPP
#define CCXLOADSTRAJECTORYCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory create command lets you create your desired Trajectory Loads
 */
class ccxLoadsTrajectoryCreateCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryCreateCommand();
  ~ccxLoadsTrajectoryCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYCREATECOMMAND_HPP