#ifndef CCXLOADSGRAVITYCREATECOMMAND_HPP
#define CCXLOADSGRAVITYCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Gravity create command lets you create your desired Gravity Loads
 */
class ccxLoadsGravityCreateCommand : public CubitCommand
{
public:
  ccxLoadsGravityCreateCommand();
  ~ccxLoadsGravityCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSGRAVITYCREATECOMMAND_HPP