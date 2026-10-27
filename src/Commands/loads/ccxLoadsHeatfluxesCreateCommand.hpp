#ifndef CCXLOADSHEATFLUXESCREATECOMMAND_HPP
#define CCXLOADSHEATFLUXESCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Heatflux create command lets you create your desired Heatflux Loads
 */
class ccxLoadsHeatfluxesCreateCommand : public CubitCommand
{
public:
  ccxLoadsHeatfluxesCreateCommand();
  ~ccxLoadsHeatfluxesCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSHEATFLUXESCREATECOMMAND_HPP