#ifndef CCXLOADSCENTRIFUGALCREATECOMMAND_HPP
#define CCXLOADSCENTRIFUGALCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Centrifugal create command lets you create your desired Centrifugal Loads
 */
class ccxLoadsCentrifugalCreateCommand : public CubitCommand
{
public:
  ccxLoadsCentrifugalCreateCommand();
  ~ccxLoadsCentrifugalCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSCENTRIFUGALCREATECOMMAND_HPP