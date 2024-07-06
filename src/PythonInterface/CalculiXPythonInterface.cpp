#include "CalculiXPythonInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

CalculiXPythonInterface::CalculiXPythonInterface()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  std::string log = "CalculiX Python Interface loaded!\n";
  PRINT_INFO("%s", log.c_str());
}

CalculiXPythonInterface::~CalculiXPythonInterface()
{
  delete ccx_iface;
}

void CalculiXPythonInterface::help()
{
  std::string log = "CalculiX Python Interface HELP:\n";
  log.append("Interactions with Jobs\n");
  log.append("int get_job_status(int job_id); //status -2 no job -1 no process, 1 process running, 2 process finished, 3 process killed, 4 process finished with errors, 5 process running currently waiting \n");
  log.append("Interactions with FRD Results:\n");
  log.append("std::vector<std::string> frd_get_result_block_types(int job_id); // returns a list of all result block types \n");
  log.append("std::vector<std::string> frd_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type\n");
  log.append("std::vector<int> frd_get_total_increments(int job_id); // returns a list of the total increments\n");
  log.append("double frd_get_time_from_total_increment(int job_id, int total_increment); // returns a the time for a total increment\n");
  log.append("std::vector<int> frd_get_node_ids_between_limits(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_limit,double upper_limit); // returns the global node ids within the limit\n");
  log.append("std::vector<int> frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global node ids smaller than the value\n");
  log.append("std::vector<int> frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global node ids greater than the value\n");
  log.append("std::vector<int> frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value); // returns the global element ids within the values\n");
  log.append("std::vector<int> frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global element ids smaller than the values\n");
  log.append("std::vector<int> frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global element ids greater than the values\n");
  log.append("std::vector<int> frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit); // returns the global element ids where the largest difference between nodal values exceeds the limit\n");
  log.append("double frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component); // returns the queried node_id value or zero if no value exists\n");
  log.append("std::vector<double> frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type); // returns the queried node_id values\n");
  log.append("Interactions with DAT Results:\n");
  log.append("std::vector<std::string> dat_get_result_block_types(int job_id); // returns a list of all result block types\n");
  log.append("std::vector<std::string> dat_get_result_block_set(int job_id); // returns a list of all result block set\n");
  log.append("std::vector<std::string> dat_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type and set\n");
  log.append("std::vector<double> dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set); // returns a list of all result block components for a block type and set\n");
  log.append("std::vector<int> dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value); // returns the global node ids within the values\n");
  log.append("std::vector<int> dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global node ids smaller than the value\n");
  log.append("std::vector<int> dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global node ids greater than the value\n");
  log.append("std::vector<int> dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value); // returns the global element ids within the values\n");
  log.append("std::vector<int> dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global element ids smaller than the values\n");
  log.append("std::vector<int> dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global element ids greater than the values  \n");
  log.append("std::vector<int> dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit); // returns the global element ids where the largest difference between element values exceeds the limit\n");
  log.append("double dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component); // returns the queried node_id value or zero if no value exists\n");
  log.append("std::vector<double> dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set); // returns the queried node_id values\n");
  log.append("std::vector<double> dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component); // returns the queried element integration point values for a component\n");
  log.append("std::vector<std::vector<double>> dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set); // returns the queried element integration point values for all components\n");
  log.append("std::vector<std::vector<std::vector<double>>> dat_get_buckle(int job_id); // returns the buckling data for a job\n");
  PRINT_INFO("%s", log.c_str());
}

int CalculiXPythonInterface::get_job_status(int job_id)
{
  return ccx_iface->get_job_status(job_id);
}

std::vector<std::string> CalculiXPythonInterface::frd_get_result_block_types(int job_id)
{
  return ccx_iface->frd_get_result_block_types(job_id);
}

std::vector<std::string> CalculiXPythonInterface::frd_get_result_block_components(int job_id, std::string result_block_type)
{
  return ccx_iface->frd_get_result_block_components(job_id, result_block_type);
}

std::vector<int> CalculiXPythonInterface::frd_get_total_increments(int job_id)
{
  return ccx_iface->frd_get_total_increments(job_id);
}

double CalculiXPythonInterface::frd_get_time_from_total_increment(int job_id, int total_increment)
{
  return ccx_iface->frd_get_time_from_total_increment(job_id,total_increment);
}

std::vector<int> CalculiXPythonInterface::frd_get_node_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  return ccx_iface->frd_get_node_ids_between_values(job_id,total_increment,result_block_type,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXPythonInterface::frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_iface->frd_get_node_ids_smaller_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_iface->frd_get_node_ids_greater_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  return ccx_iface->frd_get_element_ids_between_values(job_id,total_increment,result_block_type,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXPythonInterface::frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_iface->frd_get_element_ids_smaller_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_iface->frd_get_element_ids_greater_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit)
{
  return ccx_iface->frd_get_element_ids_over_limit(job_id,total_increment,result_block_type,result_block_component,limit);
}

double CalculiXPythonInterface::frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component)
{
  return ccx_iface->frd_get_node_value(job_id,node_id,total_increment,result_block_type,result_block_component);
}

std::vector<double> CalculiXPythonInterface::frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type)
{
  return ccx_iface->frd_get_node_values(job_id,node_id,total_increment,result_block_type);
}

std::vector<std::string> CalculiXPythonInterface::dat_get_result_block_types(int job_id)
{
  return ccx_iface->dat_get_result_block_types(job_id);
}

std::vector<std::string> CalculiXPythonInterface::dat_get_result_block_set(int job_id)
{
  return ccx_iface->dat_get_result_block_set(job_id);
}

std::vector<std::string> CalculiXPythonInterface::dat_get_result_block_components(int job_id, std::string result_block_type)
{
  return ccx_iface->dat_get_result_block_components(job_id, result_block_type);
}

std::vector<double> CalculiXPythonInterface::dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set)
{
  return ccx_iface->dat_get_result_block_times(job_id, result_block_type, result_block_set);
}

std::vector<int> CalculiXPythonInterface::dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  return ccx_iface->dat_get_node_ids_between_values(job_id,time,result_block_type,result_block_set,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXPythonInterface::dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return ccx_iface->dat_get_node_ids_smaller_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return ccx_iface->dat_get_node_ids_greater_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  return ccx_iface->dat_get_element_ids_between_values(job_id,time,result_block_type,result_block_set,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXPythonInterface::dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return ccx_iface->dat_get_element_ids_smaller_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return ccx_iface->dat_get_element_ids_greater_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXPythonInterface::dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit)
{
  return ccx_iface->dat_get_element_ids_over_limit(job_id,time,result_block_type,result_block_set,result_block_component,limit);
}

double CalculiXPythonInterface::dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  return ccx_iface->dat_get_node_value(job_id,node_id,time,result_block_type,result_block_set,result_block_component);
}

std::vector<double> CalculiXPythonInterface::dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set)
{
  return ccx_iface->dat_get_node_values(job_id,node_id,time,result_block_type,result_block_set);
}

std::vector<double> CalculiXPythonInterface::dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  return ccx_iface->dat_get_element_values_for_component(job_id,element_id,time,result_block_type,result_block_set,result_block_component);
}

std::vector<std::vector<double>> CalculiXPythonInterface::dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set)
{
  return ccx_iface->dat_get_element_values(job_id,element_id,time,result_block_type,result_block_set);
}

std::vector<std::vector<std::vector<double>>> CalculiXPythonInterface::dat_get_buckle(int job_id)
{
  return ccx_iface->dat_get_buckle(job_id);
}