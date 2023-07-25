#ifndef CCXSURFACEINTERACTIONEXPONENTIALCREATECOMMAND_HPP
#define CCXSURFACEINTERACTIONEXPONENTIALCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... create command lets you create your desired Surface Interactions
 */
class ccxSurfaceInteractionExponentialCreateCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionExponentialCreateCommand();
  ~ccxSurfaceInteractionExponentialCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONEXPONENTIALCREATECOMMAND_HPP
