#ifndef CCXHISTORYOUTPUTNODEMODIFYCOMMAND_HPP
#define CCXHISTORYOUTPUTNODEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxHistoryOutputNodeModifyCommand : public CubitCommand
{
public:
  ccxHistoryOutputNodeModifyCommand();
  ~ccxHistoryOutputNodeModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHISTORYOUTPUTNODEMODIFYCOMMAND_HPP