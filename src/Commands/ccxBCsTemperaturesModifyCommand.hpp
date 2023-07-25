#ifndef CCXBCSTEMPERATURESMODIFYCOMMAND_HPP
#define CCXBCSTEMPERATURESMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The BCs modify command lets you modify your desired BCs
 */
class ccxBCsTemperaturesModifyCommand : public CubitCommand
{
public:
  ccxBCsTemperaturesModifyCommand();
  ~ccxBCsTemperaturesModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBCSTEMPERATURESMODIFYCOMMAND_HPP