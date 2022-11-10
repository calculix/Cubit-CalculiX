#ifndef CALCULIXCOREINTERFACE_HPP
#define CALCULIXCOREINTERFACE_HPP

class CalculiXCoreInterface
{

public:
  CalculiXCoreInterface();
  ~CalculiXCoreInterface();
	
  std::string print_data();
  bool log_str(std::string str_log);
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  bool core_update(); // lets the core check for updates aka changes from the entities
  bool core_reset(); // reset the whole core to the init level
};

#endif // CALCULIXCOREINTERFACE_HPP
