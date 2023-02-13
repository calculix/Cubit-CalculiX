#ifndef CCXCONTACTPAIRDELETECOMMAND_HPP
#define CCXCONTACTPAIRDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The contact pair delete command lets you delete your desired contact pairs
 */
class ccxContactPairDeleteCommand : public CubitCommand
{
public:
  ccxContactPairDeleteCommand();
  ~ccxContactPairDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXCONTACTPAIRDELETECOMMAND_HPP
