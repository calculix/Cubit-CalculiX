#ifndef CCXSURFACEINTERACTIONTABULARMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONTABULARMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionTabularModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionTabularModifyCommand();
  ~ccxSurfaceInteractionTabularModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONTABULARMODIFYCOMMAND_HPP
