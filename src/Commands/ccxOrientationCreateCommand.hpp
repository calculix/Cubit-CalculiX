#ifndef CCXORIENTATIONCREATECOMMAND_HPP
#define CCXORIENTATIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Orientation create command lets you create your desired Orientations
 */
class ccxOrientationCreateCommand : public CubitCommand
{
public:
  ccxOrientationCreateCommand();
  ~ccxOrientationCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXORIENTATIONCREATECOMMAND_HPP
