#ifndef CCXSURFACEINTERACTIONEXPONENTIALMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONEXPONENTIALMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionExponentialModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionExponentialModifyCommand();
  ~ccxSurfaceInteractionExponentialModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONEXPONENTIALMODIFYCOMMAND_HPP
