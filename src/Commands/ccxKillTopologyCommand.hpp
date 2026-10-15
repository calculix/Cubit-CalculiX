#ifndef CCXKILLTOPOLOGYCOMMAND_HPP
#define CCXKILLTOPOLOGYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The kill Job command lets you kill your desired Jobs
 */
class ccxKillTopologyCommand : public CubitCommand
{
public:
  ccxKillTopologyCommand();
  ~ccxKillTopologyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXKILLTOPOLOGYCOMMAND_HPP