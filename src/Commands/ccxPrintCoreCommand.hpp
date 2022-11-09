#ifndef CCXPRINTCORECOMMAND_HPP
#define CCXPRINTCORECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The ccxVersionCommand class reimplements the "version" command to
 * display information about the version of CalculixPlugin in addition to the
 * version information displayed by Cubit.
 */
class ccxPrintCoreCommand : public CubitCommand
{
public:
  ccxPrintCoreCommand();
  ~ccxPrintCoreCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXPRINTCORECOMMAND_HPP
