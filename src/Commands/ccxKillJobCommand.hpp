#ifndef CCXKILLJOBCOMMAND_HPP
#define CCXKILLJOBCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The kill Job command lets you kill your desired Jobs
 */
class ccxKillJobCommand : public CubitCommand
{
public:
  ccxKillJobCommand();
  ~ccxKillJobCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXKILLJOBCOMMAND_HPP