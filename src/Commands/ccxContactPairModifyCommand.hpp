#ifndef CCXCONTACTPAIRMODIFYCOMMAND_HPP
#define CCXCONTACTPAIRMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Contact Pair modify command lets you modify your desired Contact Pairs
 */
class ccxContactPairModifyCommand : public CubitCommand
{
public:
  ccxContactPairModifyCommand();
  ~ccxContactPairModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONTACTPAIRMODIFYCOMMAND_HPP
