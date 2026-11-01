#ifndef CCXLOADSFORCESDELETECOMMAND_HPP
#define CCXLOADSFORCESDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Force delete command lets you delete your desired Force Loads
 */
class ccxLoadsForcesDeleteCommand : public CubitCommand
{
public:
  ccxLoadsForcesDeleteCommand();
  ~ccxLoadsForcesDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSFORCESDELETECOMMAND_HPP