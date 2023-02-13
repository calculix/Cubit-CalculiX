#ifndef CCXCONTACTPAIRCREATECOMMAND_HPP
#define CCXCONTACTPAIRCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Contact Pair create command lets you create your desired Contact Pairs
 */
class ccxContactPairCreateCommand : public CubitCommand
{
public:
  ccxContactPairCreateCommand();
  ~ccxContactPairCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONTACTPAIRCREATECOMMAND_HPP
