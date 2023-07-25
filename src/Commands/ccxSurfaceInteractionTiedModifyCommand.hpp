#ifndef CCXSURFACEINTERACTIONTIEDMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONTIEDMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionTiedModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionTiedModifyCommand();
  ~ccxSurfaceInteractionTiedModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONTIEDMODIFYCOMMAND_HPP
