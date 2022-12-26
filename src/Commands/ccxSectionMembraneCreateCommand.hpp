#ifndef CCXSECTIONMEMBRANECREATECOMMAND_HPP
#define CCXSECTIONMEMBRANECREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section create command lets you create your desired sections
 */
class ccxSectionMembraneCreateCommand : public CubitCommand
{
public:
  ccxSectionMembraneCreateCommand();
  ~ccxSectionMembraneCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONMEMBRANECREATECOMMAND
