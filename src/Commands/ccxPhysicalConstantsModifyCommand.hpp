#ifndef CCXPHYSICALCONSTANTSMODIFYCOMMAND_HPP
#define CCXPHYSICALCONSTANTSMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Physical Constants modify command lets you modify your desired Physical Constants
 */
class ccxPhysicalConstantsModifyCommand : public CubitCommand
{
public:
  ccxPhysicalConstantsModifyCommand();
  ~ccxPhysicalConstantsModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXPHYSICALCONSTANTSMODIFYCOMMAND_HPP