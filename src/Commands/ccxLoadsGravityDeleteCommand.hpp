#ifndef CCXLOADSGRAVITYDELETECOMMAND_HPP
#define CCXLOADSGRAVITYDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Gravity delete command lets you delete your desired Gravity Loads
 */
class ccxLoadsGravityDeleteCommand : public CubitCommand
{
public:
  ccxLoadsGravityDeleteCommand();
  ~ccxLoadsGravityDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSGRAVITYDELETECOMMAND_HPP