#ifndef CCXSECTIONSHELLCREATECOMMAND_HPP
#define CCXSECTIONSHELLCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section create command lets you create your desired sections
 */
class ccxSectionShellCreateCommand : public CubitCommand
{
public:
  ccxSectionShellCreateCommand();
  ~ccxSectionShellCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONSHELLCREATECOMMAND
