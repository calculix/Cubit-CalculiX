#ifndef CCXSURFACEINTERACTIONLINEARCREATECOMMAND_HPP
#define CCXSURFACEINTERACTIONLINEARCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... create command lets you create your desired Surface Interactions
 */
class ccxSurfaceInteractionLinearCreateCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionLinearCreateCommand();
  ~ccxSurfaceInteractionLinearCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONLINEARCREATECOMMAND_HPP
