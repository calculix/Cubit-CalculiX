#ifndef CCXWAITJOBCOMMAND_HPP
#define CCXWAITJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Run Job command lets you run your desired Jobs
 */
class ccxWaitJobCommand : public CubitCommand
{
public:
  ccxWaitJobCommand();
  ~ccxWaitJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXWAITJOBCOMMAND_HPP