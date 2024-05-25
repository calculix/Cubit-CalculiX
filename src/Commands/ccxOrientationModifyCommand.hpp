#ifndef CCXORIENTATIONMODIFYCOMMAND_HPP
#define CCXORIENTATIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Orientation modify command lets you modify your desired Orientations
 */
class ccxOrientationModifyCommand : public CubitCommand
{
public:
  ccxOrientationModifyCommand();
  ~ccxOrientationModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXORIENTATIONMODIFYCOMMAND_HPP