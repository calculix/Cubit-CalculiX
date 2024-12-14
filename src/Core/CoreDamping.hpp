#ifndef COREDAMPING_HPP
#define COREDAMPING_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreDamping
{

public:
  CoreDamping();
  ~CoreDamping();

  std::vector<std::string> damping_data;
  // damping_data[0] alpha
  // damping_data[1] beta
  
  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the customlines
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool modify_damping(std::vector<std::string> options, std::vector<int> options_marker); // modify damping
  bool delete_damping(bool delete_alpha, bool delete_beta); // deletes the values
  std::string get_damping_export_data(); // get damping export
  std::string print_data(); // prints out the customlines_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREDAMPING_HPP