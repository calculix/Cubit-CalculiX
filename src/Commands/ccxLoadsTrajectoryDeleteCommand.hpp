#ifndef CCXLOADSTRAJECTORYDELETECOMMAND_HPP
#define CCXLOADSTRAJECTORYDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Trajectory delete command lets you delete your desired Trajectory Loads
 */
class ccxLoadsTrajectoryDeleteCommand : public CubitCommand
{
public:
  ccxLoadsTrajectoryDeleteCommand();
  ~ccxLoadsTrajectoryDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSTRAJECTORYDELETECOMMAND_HPP