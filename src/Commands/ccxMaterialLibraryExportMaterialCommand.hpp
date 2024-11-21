#ifndef CCXMATERIALLIBRARYEXPORTMATERIALCOMMAND_HPP
#define CCXMATERIALLIBRARYEXPORTMATERIALCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will export a cubit material into the material library
 */
class ccxMaterialLibraryExportMaterialCommand : public CubitCommand
{
public:
  ccxMaterialLibraryExportMaterialCommand();
  ~ccxMaterialLibraryExportMaterialCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYEXPORTMATERIALCOMMAND_HPP