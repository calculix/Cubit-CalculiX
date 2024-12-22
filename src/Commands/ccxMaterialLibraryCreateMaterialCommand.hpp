#ifndef CCXMATERIALLIBRARYCREATEMATERIALCOMMAND_HPP
#define CCXMATERIALLIBRARYCREATEMATERIALCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Command will create a new material in a group in the material library
 */
class ccxMaterialLibraryCreateMaterialCommand : public CubitCommand
{
public:
  ccxMaterialLibraryCreateMaterialCommand();
  ~ccxMaterialLibraryCreateMaterialCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXMATERIALLIBRARYCREATEMATERIALCOMMAND_HPP