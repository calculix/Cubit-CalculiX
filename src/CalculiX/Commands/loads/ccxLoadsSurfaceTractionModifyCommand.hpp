#ifndef CCXLOADSSURFACETRACTIONMODIFYCOMMAND_HPP
#define CCXLOADSSURFACETRACTIONMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Surface Traction modify command lets you modify your desired Surface Traction Loads
 */
class ccxLoadsSurfaceTractionModifyCommand : public CubitCommand
{
public:
  ccxLoadsSurfaceTractionModifyCommand();
  ~ccxLoadsSurfaceTractionModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSSURFACETRACTIONMODIFYCOMMAND_HPP