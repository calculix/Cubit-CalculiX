#ifndef CCXLOADSBODYHEATFLUXCREATECOMMAND_HPP
#define CCXLOADSBODYHEATFLUXCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Gravity create command lets you create your desired Gravity Loads
 */
class ccxLoadsBodyHeatfluxCreateCommand : public CubitCommand
{
public:
  ccxLoadsBodyHeatfluxCreateCommand();
  ~ccxLoadsBodyHeatfluxCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSBODYHEATFLUXCREATECOMMAND_HPP