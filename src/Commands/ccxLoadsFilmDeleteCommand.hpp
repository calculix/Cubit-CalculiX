#ifndef CCXLOADSFILMDELETECOMMAND_HPP
#define CCXLOADSFILMDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Loads Film delete command lets you delete your desired Film Loads
 */
class ccxLoadsFilmDeleteCommand : public CubitCommand
{
public:
  ccxLoadsFilmDeleteCommand();
  ~ccxLoadsFilmDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXLOADSFILMDELETECOMMAND_HPP