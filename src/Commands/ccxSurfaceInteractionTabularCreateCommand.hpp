#ifndef CCXSURFACEINTERACTIONTABULARCREATECOMMAND_HPP
#define CCXSURFACEINTERACTIONTABULARCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... create command lets you create your desired Surface Interactions
 */
class ccxSurfaceInteractionTabularCreateCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionTabularCreateCommand();
  ~ccxSurfaceInteractionTabularCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONTABULARCREATECOMMAND_HPP
