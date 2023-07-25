#ifndef CCXSECTIONSOLIDMODIFYCOMMAND_HPP
#define CCXSECTIONSOLIDMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section modify command lets you modify your desired section
 */
class ccxSectionSolidModifyCommand : public CubitCommand
{
public:
  ccxSectionSolidModifyCommand();
  ~ccxSectionSolidModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONSOLIDMODIFYCOMMAND
