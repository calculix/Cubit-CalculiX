#ifndef CCXLOADSGRAVITYMODIFYCOMMAND_HPP
#define CCXLOADSGRAVITYMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Gravity modify command lets you modify your desired Gravity Loads
 */
class ccxLoadsGravityModifyCommand : public CubitCommand
{
public:
  ccxLoadsGravityModifyCommand();
  ~ccxLoadsGravityModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSGRAVITYMODIFYCOMMAND_HPP