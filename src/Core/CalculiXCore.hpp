#ifndef CALCULIXCORE_HPP
#define CALCULIXCORE_HPP

#include "CoreBlocks.hpp"

class CalculiXCore
{
  CoreBlocks cb;

public:
  CalculiXCore();
  ~CalculiXCore();

  bool print_to_log(std::string str_log);
  bool init();
  std::string print_data();
};

#endif // CALCULIXCore_HPP
