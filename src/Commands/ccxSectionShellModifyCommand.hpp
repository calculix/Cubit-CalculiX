#ifndef CCXSECTIONSHELLMODIFYCOMMAND_HPP
#define CCXSECTIONSHELLMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section modify command lets you modify your desired section
 */
class ccxSectionShellModifyCommand : public CubitCommand
{
public:
  ccxSectionShellModifyCommand();
  ~ccxSectionShellModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONSHELLMODIFYCOMMAND_HPP
