#ifndef CCXLOADSPRESSURESCREATECOMMAND_HPP
#define CCXLOADSPRESSURESCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Pressure create command lets you create your desired Pressure Loads
 */
class ccxLoadsPressuresCreateCommand : public CubitCommand
{
public:
  ccxLoadsPressuresCreateCommand();
  ~ccxLoadsPressuresCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSPRESSURESCREATECOMMAND_HPP