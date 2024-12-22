#ifndef CCXLOADSFILMCREATECOMMAND_HPP
#define CCXLOADSFILMCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Film create command lets you create your desired Film Loads
 */
class ccxLoadsFilmCreateCommand : public CubitCommand
{
public:
  ccxLoadsFilmCreateCommand();
  ~ccxLoadsFilmCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSFILMCREATECOMMAND_HPP