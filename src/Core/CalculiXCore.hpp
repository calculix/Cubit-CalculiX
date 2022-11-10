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
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types;
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
};

#endif // CALCULIXCore_HPP
