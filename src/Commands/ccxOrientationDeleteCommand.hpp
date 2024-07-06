#ifndef CCXORIENTATIONDELETECOMMAND_HPP
#define CCXORIENTATIONDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The orientation delete command lets you delete your desired orientations
 */
class ccxOrientationDeleteCommand : public CubitCommand
{
public:
  ccxOrientationDeleteCommand();
  ~ccxOrientationDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXORIENTATIONDELETECOMMAND_HPP
