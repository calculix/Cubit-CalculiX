#ifndef CCXHISTORYOUTPUTSECTIONMODIFYCOMMAND_HPP
#define CCXHISTORYOUTPUTSECTIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output modify command lets you modify your desired Outputs
 */
class ccxHistoryOutputSectionModifyCommand : public CubitCommand
{
public:
  ccxHistoryOutputSectionModifyCommand();
  ~ccxHistoryOutputSectionModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHISTORYOUTPUTSECTIONMODIFYCOMMAND_HPP