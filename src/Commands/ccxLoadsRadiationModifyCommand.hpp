#ifndef CCXLOADSRADIATIONMODIFYCOMMAND_HPP
#define CCXLOADSRADIATIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Radiation modify command lets you modify your desired Radiation Loads
 */
class ccxLoadsRadiationModifyCommand : public CubitCommand
{
public:
  ccxLoadsRadiationModifyCommand();
  ~ccxLoadsRadiationModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSRADIATIONMODIFYCOMMAND_HPP