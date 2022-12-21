#ifndef CCXSECTIONCREATECOMMAND_HPP
#define CCXSECTIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section create command lets you create your desired sections
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
