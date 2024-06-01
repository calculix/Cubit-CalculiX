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

*/

class CalculiXCoreInterface;

class CalculiXPythonInterface
{

public:
  CalculiXPythonInterface();
  ~CalculiXPythonInterface();
	
  void help(); // prints out python interface help
  //JOBS
  int get_job_status(int job_id); //status -2 no job -1 no process, 1 process running, 2 process finished, 3 process killed, 4 process finished with errors, 5 process running currently waiting
  //QUERY results
  //FRD results
  std::vector<std::string> frd_get_result_block_types(int job_id); // returns a list of all result block types
  std::vector<std::string> frd_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type
  std::vector<int> frd_get_total_increments(int job_id); // returns a list of the total increments
  double frd_get_time_from_total_increment(int job_id, int total_increment); // returns a the time for a total increment
  std::vector<int> frd_get_node_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value); // returns the global node ids within the values
  std::vector<int> frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global node ids smaller than the value
  std::vector<int> frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global node ids greater than the value
  std::vector<int> frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value); // returns the global element ids within the values
  std::vector<int> frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global element ids smaller than the values
  std::vector<int> frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global element ids greater than the values
  std::vector<int> frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit); // returns the global element ids where the largest difference between nodal values exceeds the limit
  double frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component); // returns the queried node_id value or zero if no value exists
  std::vector<double> frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type); // returns the queried node_id values
  //DAT results
  std::vector<std::string> dat_get_result_block_types(int job_id); // returns a list of all result block types
  std::vector<std::string> dat_get_result_block_set(int job_id); // returns a list of all result block set
  std::vector<std::string> dat_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type and set
  std::vector<double> dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set); // returns a list of all result block components for a block type and set
  std::vector<int> dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value); // returns the global node ids within the values
  std::vector<int> dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global node ids smaller than the value
  std::vector<int> dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global node ids greater than the value
  std::vector<int> dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value); // returns the global element ids within the values
  std::vector<int> dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global element ids smaller than the values
  std::vector<int> dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global element ids greater than the values  
  std::vector<int> dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit); // returns the global element ids where the largest difference between element values exceeds the limit
  double dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component); // returns the queried node_id value or zero if no value exists
  std::vector<double> dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set); // returns the queried node_id values
  std::vector<double> dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component); // returns the queried element integration point values for a component
  std::vector<std::vector<double>> dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set); // returns the queried element integration point values for all components

  CalculiXCoreInterface *ccx_iface;
};

#endif // CALCULIXPYTHONINTERFACE_HPP
