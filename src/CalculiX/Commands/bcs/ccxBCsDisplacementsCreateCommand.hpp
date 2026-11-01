#ifndef CCXBCSDISPLACEMENTSCREATECOMMAND_HPP
#define CCXBCSDISPLACEMENTSCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The displacement create command
 */
class ccxBCsDisplacementsCreateCommand : public CubitCommand
{
public:
  ccxBCsDisplacementsCreateCommand();
  ~ccxBCsDisplacementsCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXBCSDISPLACEMENTSCREATECOMMAND_HPP