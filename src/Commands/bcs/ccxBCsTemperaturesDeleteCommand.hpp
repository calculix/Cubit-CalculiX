#ifndef CCXBCSTEMPERATURESDELETECOMMAND_HPP
#define CCXBCSTEMPERATURESDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The temperature delete command
 */
class ccxBCsTemperaturesDeleteCommand : public CubitCommand
{
public:
  ccxBCsTemperaturesDeleteCommand();
  ~ccxBCsTemperaturesDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBCSTEMPERATURESDELETECOMMAND_HPP