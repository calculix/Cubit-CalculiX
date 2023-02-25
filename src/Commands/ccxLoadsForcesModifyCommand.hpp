#ifndef CCXLOADSFORCESMODIFYCOMMAND_HPP
#define CCXLOADSFORCESMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads modify command lets you modify your desired Loads
 */
class ccxLoadsForcesModifyCommand : public CubitCommand
{
public:
  ccxLoadsForcesModifyCommand();
  ~ccxLoadsForcesModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSFORCESMODIFYCOMMAND_HPP