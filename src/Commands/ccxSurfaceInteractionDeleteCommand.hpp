#ifndef CCXSURFACEINTERACTIONDELETECOMMAND_HPP
#define CCXSURFACEINTERACTIONDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The surface interaction delete command lets you delete your desired surface interactions
 */
class ccxSurfaceInteractionDeleteCommand : public CubitCommand
{
public:
  ccxSurfaceInteractionDeleteCommand();
  ~ccxSurfaceInteractionDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXSURFACEINTERACTIONDELETECOMMAND_HPP
