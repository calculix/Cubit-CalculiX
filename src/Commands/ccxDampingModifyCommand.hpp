#ifndef CCXDAMPINGMODIFYCOMMAND_HPP
#define CCXDAMPINGMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Damping modify command lets you modify your desired Damping
 */
class ccxDampingModifyCommand : public CubitCommand
{
public:
  ccxDampingModifyCommand();
  ~ccxDampingModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXDAMPINGMODIFYCOMMAND_HPP