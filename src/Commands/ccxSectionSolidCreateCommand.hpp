#ifndef CCXSECTIONSOLIDCREATECOMMAND_HPP
#define CCXSECTIONSOLIDCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section create command lets you create your desired sections
 */
class ccxSectionSolidCreateCommand : public CubitCommand
{
public:
  ccxSectionSolidCreateCommand();
  ~ccxSectionSolidCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONSOLIDCREATECOMMAND
