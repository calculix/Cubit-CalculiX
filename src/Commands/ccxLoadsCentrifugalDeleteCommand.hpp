#ifndef CCXLOADSCENTRIFUGALDELETECOMMAND_HPP
#define CCXLOADSCENTRIFUGALDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Centrifugal delete command lets you delete your desired Centrifugal Loads
 */
class ccxLoadsCentrifugalDeleteCommand : public CubitCommand
{
public:
  ccxLoadsCentrifugalDeleteCommand();
  ~ccxLoadsCentrifugalDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSCENTRIFUGALDELETECOMMAND_HPP