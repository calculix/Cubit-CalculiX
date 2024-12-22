#ifndef CCXLOADSRADIATIONCREATECOMMAND_HPP
#define CCXLOADSRADIATIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Radiation create command lets you create your desired Radiation Loads
 */
class ccxLoadsRadiationCreateCommand : public CubitCommand
{
public:
  ccxLoadsRadiationCreateCommand();
  ~ccxLoadsRadiationCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSRADIATIONCREATECOMMAND_HPP