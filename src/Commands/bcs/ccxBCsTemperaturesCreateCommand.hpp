#ifndef CCXBCSTEMPERATURESCREATECOMMAND_HPP
#define CCXBCSTEMPERATURESCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Temperature create command lets you create a plugin-owned temperature BC
 */
class ccxBCsTemperaturesCreateCommand : public CubitCommand
{
public:
  ccxBCsTemperaturesCreateCommand();
  ~ccxBCsTemperaturesCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBCSTEMPERATURESCREATECOMMAND_HPP