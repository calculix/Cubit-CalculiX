#ifndef CCXRESETCOMMAND_HPP
#define CCXRESETCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The ccxResetCommand class resets the ccx core.
 */
class ccxResetCommand : public CubitCommand
{
public:
  ccxResetCommand();
  ~ccxResetCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRESETCOMMAND_HPP
