#ifndef CCXAMPLITUDECREATECOMMAND_HPP
#define CCXAMPLITUDECREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Amplitude create command lets you create your desired Amplitudes
 */
class ccxAmplitudeCreateCommand : public CubitCommand
{
public:
  ccxAmplitudeCreateCommand();
  ~ccxAmplitudeCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXAMPLITUDECREATECOMMAND_HPP
