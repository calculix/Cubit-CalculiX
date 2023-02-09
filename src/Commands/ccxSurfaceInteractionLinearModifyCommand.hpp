#ifndef CCXSURFACEINTERACTIONLINEARMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONLINEARMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionLinearModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionLinearModifyCommand();
  ~ccxSurfaceInteractionLinearModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONLINEARMODIFYCOMMAND_HPP
