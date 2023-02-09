#ifndef CCXSURFACEINTERACTIONNAMEMODIFYCOMMAND_HPP
#define CCXSURFACEINTERACTIONNAMEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The SurfaceInteraction... modify command lets you modify your desired Surface Interactions
 */
class ccxSurfaceInteractionNameModifyCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionNameModifyCommand();
  ~ccxSurfaceInteractionNameModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONNAMEMODIFYCOMMAND_HPP
