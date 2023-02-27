#ifndef CCXHISTORYOUTPUTCONTACTMODIFYCOMMAND_HPP
#define CCXHISTORYOUTPUTCONTACTMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxHistoryOutputContactModifyCommand : public CubitCommand
{
public:
  ccxHistoryOutputContactModifyCommand();
  ~ccxHistoryOutputContactModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHISTORYOUTPUTCONTACTMODIFYCOMMAND_HPP