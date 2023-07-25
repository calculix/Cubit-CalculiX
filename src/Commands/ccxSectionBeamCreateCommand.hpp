#ifndef CCXSECTIONBEAMCREATECOMMAND_HPP
#define CCXSECTIONBEAMCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Section create command lets you create your desired sections
 */
class ccxSectionBeamCreateCommand : public CubitCommand
{
public:
  ccxSectionBeamCreateCommand();
  ~ccxSectionBeamCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSECTIONBEAMCREATECOMMAND_HPP
