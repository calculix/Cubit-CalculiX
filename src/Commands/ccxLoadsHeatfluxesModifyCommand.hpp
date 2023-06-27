#ifndef CCXLOADSHEATFLUXESMODIFYCOMMAND_HPP
#define CCXLOADSHEATFLUXESMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads modify command lets you modify your desired Loads
 */
class ccxLoadsHeatfluxesModifyCommand : public CubitCommand
{
public:
  ccxLoadsHeatfluxesModifyCommand();
  ~ccxLoadsHeatfluxesModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSHEATFLUXESMODIFYCOMMAND_HPP