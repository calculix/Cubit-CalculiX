#ifndef CALCULIXPYTHONINTERFACE_HPP
#define CALCULIXPYTHONINTERFACE_HPP

#include <vector>
#include <string>

/* don't forget to wrap with 'swig -python -py3 -c++ CalculiXPythonInterface.i'  

and if compiling fails because of qt replace Python.h with 

#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")

#!python
ccx.frd_get_result_block_types(1)
ccx.frd_get_result_block_components(1, "DISP")
*/

class CalculiXCoreInterface;

class CalculiXPythonInterface
{

public:
  CalculiXPythonInterface();
  ~CalculiXPythonInterface();
	
  void help(); // prints out python interface help
    
  //QUERY results
  //FRD results
  std::vector<std::string> frd_get_result_block_types(int job_id); // returns a list of all result block types
  std::vector<std::string> frd_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type
  std::vector<int> frd_get_total_increments(int job_id); // returns a list of the total increments
  double frd_get_time_from_total_increment(int job_id, int total_increment); // returns a the time for a total increment
  std::vector<int> frd_get_node_ids_between_limits(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_limit,double upper_limit); // returns the global node ids within the limit
  double frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component); // returns the queried node_id value
  std::vector<double> frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type); // returns the queried node_id values
  //DAT results

  CalculiXCoreInterface *ccx_iface;
};

#endif // CALCULIXPYTHONINTERFACE_HPP
