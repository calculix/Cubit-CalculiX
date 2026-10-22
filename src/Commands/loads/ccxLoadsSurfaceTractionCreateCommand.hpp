#ifndef CCXLOADSSURFACETRACTIONCREATECOMMAND_HPP
#define CCXLOADSSURFACETRACTIONCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Surface Traction create command lets you create your desired Surface Traction Loads
 */
class ccxLoadsSurfaceTractionCreateCommand : public CubitCommand
{
public:
  ccxLoadsSurfaceTractionCreateCommand();
  ~ccxLoadsSurfaceTractionCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSSURFACETRACTIONCREATECOMMAND_HPP