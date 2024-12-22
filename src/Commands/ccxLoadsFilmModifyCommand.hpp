#ifndef CCXLOADSFILMMODIFYCOMMAND_HPP
#define CCXLOADSFILMMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Film modify command lets you modify your desired Film Loads
 */
class ccxLoadsFilmModifyCommand : public CubitCommand
{
public:
  ccxLoadsFilmModifyCommand();
  ~ccxLoadsFilmModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSFILMMODIFYCOMMAND_HPP