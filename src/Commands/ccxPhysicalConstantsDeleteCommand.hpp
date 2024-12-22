#ifndef CCXPHYSICALCONSTANTSDELETECOMMAND_HPP
#define CCXPHYSICALCONSTANTSDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Physical Constants delete command lets you delete your desired Physical Constants
 */
class ccxPhysicalConstantsDeleteCommand : public CubitCommand
{
public:
  ccxPhysicalConstantsDeleteCommand();
  ~ccxPhysicalConstantsDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXPHYSICALCONSTANTSDELETECOMMAND_HPP