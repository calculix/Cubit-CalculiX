#ifndef CCXMATERIALLIBRARYCREATEGROUPCOMMAND_HPP
#define CCXMATERIALLIBRARYCREATEGROUPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will create a new group in the material library
 */
class ccxMaterialLibraryCreateGroupCommand : public CubitCommand
{
public:
  ccxMaterialLibraryCreateGroupCommand();
  ~ccxMaterialLibraryCreateGroupCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYCREATEGROUPCOMMAND_HPP