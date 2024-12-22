#ifndef CCXMATERIALLIBRARYDELETEMATERIALCOMMAND_HPP
#define CCXMATERIALLIBRARYDELETEMATERIALCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will delete the material in a group in the material library
 */
class ccxMaterialLibraryDeleteMaterialCommand : public CubitCommand
{
public:
  ccxMaterialLibraryDeleteMaterialCommand();
  ~ccxMaterialLibraryDeleteMaterialCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYDELETEMATERIALCOMMAND_HPP