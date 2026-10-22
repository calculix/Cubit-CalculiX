#ifndef CCXLOADSSURFACETRACTIONYDELETECOMMAND_HPP
#define CCXLOADSSURFACETRACTIONYDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Surface Traction delete command lets you delete your desired Surface Traction Loads
 */
class ccxLoadsSurfaceTractionDeleteCommand : public CubitCommand
{
public:
  ccxLoadsSurfaceTractionDeleteCommand();
  ~ccxLoadsSurfaceTractionDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSSURFACETRACTIONYDELETECOMMAND_HPP