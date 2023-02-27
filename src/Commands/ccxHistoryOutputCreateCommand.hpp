#ifndef CCXHISTORYOUTPUTCREATECOMMAND_HPP
#define CCXHISTORYOUTPUTCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Output create command lets you create your desired Outputs
 */
class ccxHistoryOutputCreateCommand : public CubitCommand
{
public:
  ccxHistoryOutputCreateCommand();
  ~ccxHistoryOutputCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXHISTORYOUTPUTCREATECOMMAND_HPP