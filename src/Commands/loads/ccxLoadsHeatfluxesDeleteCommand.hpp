#ifndef CCXLOADSHEATFLUXESDELETECOMMAND_HPP
#define CCXLOADSHEATFLUXESDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Heatflux delete command lets you delete your desired Heatflux Loads
 */
class ccxLoadsHeatfluxesDeleteCommand : public CubitCommand
{
public:
  ccxLoadsHeatfluxesDeleteCommand();
  ~ccxLoadsHeatfluxesDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSHEATFLUXESDELETECOMMAND_HPP