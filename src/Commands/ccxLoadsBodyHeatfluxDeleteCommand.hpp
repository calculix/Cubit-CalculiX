#ifndef CCXLOADSBODYHEATFLUXDELETECOMMAND_HPP
#define CCXLOADSBODYHEATFLUXDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads BodyHeatflux delete command lets you delete your desired Gravity Loads
 */
class ccxLoadsBodyHeatfluxDeleteCommand : public CubitCommand
{
public:
  ccxLoadsBodyHeatfluxDeleteCommand();
  ~ccxLoadsBodyHeatfluxDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSBODYHEATFLUXDELETECOMMAND_HPP