#ifndef CCXSURFACEINTERACTIONHARDCREATECOMMAND_HPP
#define CCXSURFACEINTERACTIONHARDCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... create command lets you create your desired Surface Interactions
 */
class ccxSurfaceInteractionHardCreateCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionHardCreateCommand();
  ~ccxSurfaceInteractionHardCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONHARDCREATECOMMAND_HPP
