#ifndef CCXCONTACTPAIRCREATEFROMCUBITCONTACTPAIRCOMMAND_HPP
#define CCXCONTACTPAIRCREATEFROMCUBITCONTACTPAIRCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Contact Pair create from cubit contact pairs command lets you convert Contact Pairs from cubit to ccx
 */
class ccxContactPairCreateFromCubitContactPairCommand : public CubitCommand
{
public:
  ccxContactPairCreateFromCubitContactPairCommand();
  ~ccxContactPairCreateFromCubitContactPairCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONTACTPAIRCREATEFROMCUBITCONTACTPAIRCOMMAND_HPP
