#ifndef CCXMATERIALLIBRARYIMPORTMATERIALCOMMAND_HPP
#define CCXMATERIALLIBRARYIMPORTMATERIALCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will import a  material from the material library into cubit
 */
class ccxMaterialLibraryImportMaterialCommand : public CubitCommand
{
public:
  ccxMaterialLibraryImportMaterialCommand();
  ~ccxMaterialLibraryImportMaterialCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYIMPORTMATERIALCOMMAND_HPP