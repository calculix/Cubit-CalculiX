#ifndef CCXHISTORYOUTPUTDELETECOMMAND_HPP
#define CCXHISTORYOUTPUTDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output delete command lets you delete your desired Outputs
 */
class ccxHistoryOutputDeleteCommand : public CubitCommand
{
public:
  ccxHistoryOutputDeleteCommand();
  ~ccxHistoryOutputDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHISTORYOUTPUTDELETECOMMAND_HPP
