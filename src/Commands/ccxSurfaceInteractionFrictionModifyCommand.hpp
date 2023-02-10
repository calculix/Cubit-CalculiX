#ifndef CCXSURFACEINTERACTIONFRICTIONMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONFRICTIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionFrictionModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionFrictionModifyCommand();
  ~ccxSurfaceInteractionFrictionModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONFRICTIONMODIFYCOMMAND_HPP
