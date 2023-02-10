#ifndef CCXSURFACEINTERACTIONGAPHEATGENERATIONMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONGAPHEATGENERATIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionGapHeatGenerationModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionGapHeatGenerationModifyCommand();
  ~ccxSurfaceInteractionGapHeatGenerationModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONGAPHEATGENERATIONMODIFYCOMMAND_HPP
