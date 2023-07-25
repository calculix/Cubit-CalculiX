#ifndef CCXSECTIONBEAMMODIFYCOMMAND_HPP
#define CCXSECTIONBEAMMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section modify command lets you modify your desired section
 */
class ccxSectionBeamModifyCommand : public CubitCommand
{
public:
  ccxSectionBeamModifyCommand();
  ~ccxSectionBeamModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONBEAMMODIFYCOMMAND_HPP
