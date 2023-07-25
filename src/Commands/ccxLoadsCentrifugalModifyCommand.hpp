#ifndef CCXLOADSCENTRIFUGALMODIFYCOMMAND_HPP
#define CCXLOADSCENTRIFUGALMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Centrifugal modify command lets you modify your desired Centrifugal Loads
 */
class ccxLoadsCentrifugalModifyCommand : public CubitCommand
{
public:
  ccxLoadsCentrifugalModifyCommand();
  ~ccxLoadsCentrifugalModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSCENTRIFUGALMODIFYCOMMAND_HPP