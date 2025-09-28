#ifndef CCXLOADSBODYHEATFLUXMODIFYCOMMAND_HPP
#define CCXLOADSBODYHEATFLUXMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads BodyHeatflux modify command lets you modify your desired Gravity Loads
 */
class ccxLoadsBodyHeatfluxModifyCommand : public CubitCommand
{
public:
  ccxLoadsBodyHeatfluxModifyCommand();
  ~ccxLoadsBodyHeatfluxModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSBODYHEATFLUXMODIFYCOMMAND_HPP