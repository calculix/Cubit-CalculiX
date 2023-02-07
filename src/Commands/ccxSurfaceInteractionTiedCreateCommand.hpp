#ifndef CCXSURFACEINTERACTIONTIEDCREATECOMMAND_HPP
#define CCXSURFACEINTERACTIONTIEDCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... create command lets you create your desired Surface Interactions
 */
class ccxSurfaceInteractionTiedCreateCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionTiedCreateCommand();
  ~ccxSurfaceInteractionTiedCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONTIEDCREATECOMMAND_HPP
