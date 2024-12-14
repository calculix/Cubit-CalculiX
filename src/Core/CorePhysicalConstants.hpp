#ifndef COREPHYSICALCONSTANTS_HPP
#define COREPHYSICALCONSTANTS_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CorePhysicalConstants
{

public:
  CorePhysicalConstants();
  ~CorePhysicalConstants();

  std::vector<std::string> physicalconstants_data;
  // physicalconstants_data[0] ABSOLUTE ZERO
  // physicalconstants_data[1] STEFAN BOLTZMANN
  // physicalconstants_data[1] NEWTON GRAVITY
  
  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the customlines
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker); // modify damping
  bool delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity); // deletes the values
  std::string get_physicalconstants_export_data(); // get physicalconstants export
  std::string print_data(); // prints out the customlines_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREPHYSICALCONSTANTS_HPP