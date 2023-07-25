#ifndef CCXSECTIONDELETECOMMAND_HPP
#define CCXSECTIONDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section create command lets you create your desired sections
 */
class ccxSectionDeleteCommand : public CubitCommand
{
public:
  ccxSectionDeleteCommand();
  ~ccxSectionDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONDELETECOMMAND_HPP
