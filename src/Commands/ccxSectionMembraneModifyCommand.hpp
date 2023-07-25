#ifndef CCXSECTIONMEMBRANEMODIFYCOMMAND_HPP
#define CCXSECTIONMEMBRANEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section modify command lets you modify your desired section
 */
class ccxSectionMembraneModifyCommand : public CubitCommand
{
public:
  ccxSectionMembraneModifyCommand();
  ~ccxSectionMembraneModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONMEMBRANEMODIFYCOMMAND_HPP
