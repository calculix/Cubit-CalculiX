#ifndef CCXBLOCKELEMENTTYPECOMMAND_HPP
#define CCXBLOCKELEMENTTYPECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The ccxBlockElementTypeCommand class implements a command to change the ccx element type on a block.
 */

class ccxBlockElementTypeCommand : public CubitCommand
{
public:
  ccxBlockElementTypeCommand();
  ~ccxBlockElementTypeCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBLOCKELEMENTTYPECOMMAND
