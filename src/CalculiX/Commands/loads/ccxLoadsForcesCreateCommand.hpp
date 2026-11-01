#ifndef CCXLOADSFORCESCREATECOMMAND_HPP
#define CCXLOADSFORCESCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Force create command lets you create your desired Force Loads
 */
class ccxLoadsForcesCreateCommand : public CubitCommand
{
public:
  ccxLoadsForcesCreateCommand();
  ~ccxLoadsForcesCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSFORCESCREATECOMMAND_HPP