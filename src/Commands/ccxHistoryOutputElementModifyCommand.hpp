#ifndef CCXHISTORYOUTPUTELEMENTMODIFYCOMMAND_HPP
#define CCXHISTORYOUTPUTELEMENTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxHistoryOutputElementModifyCommand : public CubitCommand
{
public:
  ccxHistoryOutputElementModifyCommand();
  ~ccxHistoryOutputElementModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHISTORYOUTPUTELEMENTMODIFYCOMMAND_HPP