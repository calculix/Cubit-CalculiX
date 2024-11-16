#ifndef CCXMATERIALLIBRARYMODIFYGROUPCOMMAND_HPP
#define CCXMATERIALLIBRARYMODIFYGROUPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will modify the group in the material library
 */
class ccxMaterialLibraryModifyGroupCommand : public CubitCommand
{
public:
  ccxMaterialLibraryModifyGroupCommand();
  ~ccxMaterialLibraryModifyGroupCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYMODIFYGROUPCOMMAND_HPP