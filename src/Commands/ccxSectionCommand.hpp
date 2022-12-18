#ifndef CCXSECTIONCREATECOMMAND_HPP
#define CCXSECTIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The ccxVersionCommand class reimplements the "version" command to
 * display information about the version of CalculixPlugin in addition to the
 * version information displayed by Cubit.
 */
class ccxSectionCreateCommand : public CubitCommand
{
public:
  ccxSectionCreateCommand();
  ~ccxSectionCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONCREATECOMMAND
