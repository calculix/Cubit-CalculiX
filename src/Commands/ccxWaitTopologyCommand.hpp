#ifndef CCXWAITTOPOLOGYCOMMAND_HPP
#define CCXWAITTOPOLOGYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Run Job command lets you run your desired Jobs
 */
class ccxWaitTopologyCommand : public CubitCommand
{
public:
  ccxWaitTopologyCommand();
  ~ccxWaitTopologyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXWAITTOPOLOGYCOMMAND_HPP