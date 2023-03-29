#ifndef CCXJOBDELETECOMMAND_HPP
#define CCXJOBDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Job delete command lets you delete your desired Jobs
 */
class ccxJobDeleteCommand : public CubitCommand
{
public:
  ccxJobDeleteCommand();
  ~ccxJobDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXJOBDELETECOMMAND_HPP