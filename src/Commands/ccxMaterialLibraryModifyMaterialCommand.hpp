#ifndef CCXMATERIALLIBRARYMODIFYMATERIALCOMMAND_HPP
#define CCXMATERIALLIBRARYMODIFYMATERIALCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will modify the material in a group in the material library
 */
class ccxMaterialLibraryModifyMaterialCommand : public CubitCommand
{
public:
  ccxMaterialLibraryModifyMaterialCommand();
  ~ccxMaterialLibraryModifyMaterialCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYMODIFYMATERIALCOMMAND_HPP