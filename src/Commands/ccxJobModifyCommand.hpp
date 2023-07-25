#ifndef CCXJOBMODIFYCOMMAND_HPP
#define CCXJOBMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Job modify command lets you modify your desired Jobs
 */
class ccxJobModifyCommand : public CubitCommand
{
public:
  ccxJobModifyCommand();
  ~ccxJobModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXJOBMODIFYCOMMAND_HPP