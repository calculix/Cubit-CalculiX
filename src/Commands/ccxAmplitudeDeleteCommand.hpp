#ifndef CCXAMPLITUDEDELETECOMMAND_HPP
#define CCXAMPLITUDEDELETECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The amplitude delete command lets you delete your desired amplitudes
 */
class ccxAmplitudeDeleteCommand : public CubitCommand
{
public:
  ccxAmplitudeDeleteCommand();
  ~ccxAmplitudeDeleteCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXAMPLITUDEDELETECOMMAND_HPP
