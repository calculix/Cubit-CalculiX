#ifndef CCXAMPLITUDEMODIFYCOMMAND_HPP
#define CCXAMPLITUDEMODIFYCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Amplitude modify command lets you modify your desired Amplitudes
 */
class ccxAmplitudeModifyCommand : public CubitCommand
{
public:
  ccxAmplitudeModifyCommand();
  ~ccxAmplitudeModifyCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXAMPLITUDEMODFIYCOMMAND_HPP
