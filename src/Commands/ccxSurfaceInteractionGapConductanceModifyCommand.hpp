#ifndef CCXSURFACEINTERACTIONGAPCONDUCTANCEMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONGAPCONDUCTANCEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionGapConductanceModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionGapConductanceModifyCommand();
  ~ccxSurfaceInteractionGapConductanceModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONGAPCONDUCTANCEMODIFYCOMMAND_HPP
