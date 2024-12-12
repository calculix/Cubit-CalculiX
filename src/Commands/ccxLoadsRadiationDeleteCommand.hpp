#ifndef CCXLOADSRADIATIONDELETECOMMAND_HPP
#define CCXLOADSRADIATIONDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Radiation delete command lets you delete your desired Radiation Loads
 */
class ccxLoadsRadiationDeleteCommand : public CubitCommand
{
public:
  ccxLoadsRadiationDeleteCommand();
  ~ccxLoadsRadiationDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSRADIATIONDELETECOMMAND_HPP