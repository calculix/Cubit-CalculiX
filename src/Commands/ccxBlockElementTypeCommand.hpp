#ifndef CCXBLOCKELEMENTTYPECOMMAND_HPP
#define CCXBLOCKELEMENTTYPECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The ccxVersionCommand class reimplements the "version" command to
 * display information about the version of CalculixPlugin in addition to the
 * version information displayed by Cubit.
 */
class ccxBlockElementTypeCommand : public CubitCommand
{
public:
  ccxBlockElementTypeCommand();
  ~ccxBlockElementTypeCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBLOCKELEMENTTYPECOMMAND
