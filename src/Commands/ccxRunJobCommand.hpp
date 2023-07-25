#ifndef CCXRUNJOBCOMMAND_HPP
#define CCXRUNJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Run Job command lets you run your desired Jobs
 */
class ccxRunJobCommand : public CubitCommand
{
public:
  ccxRunJobCommand();
  ~ccxRunJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXRUNJOBCOMMAND_HPP