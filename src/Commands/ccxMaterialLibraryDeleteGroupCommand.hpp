#ifndef CCXMATERIALLIBRARYDELETEGROUPCOMMAND_HPP
#define CCXMATERIALLIBRARYDELETEGROUPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will delete the group in the material library
 */
class ccxMaterialLibraryDeleteGroupCommand : public CubitCommand
{
public:
  ccxMaterialLibraryDeleteGroupCommand();
  ~ccxMaterialLibraryDeleteGroupCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYDELETEGROUPCOMMAND_HPP