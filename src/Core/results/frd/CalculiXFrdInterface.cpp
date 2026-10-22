#include "CalculiXFrdInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<int> CalculiXFrdInterface::frd_get_nodes(int job_id)
{
  return ccx_core.frd_get_nodes(job_id);
}

bool CalculiXFrdInterface::frd_check_node_exists(int job_id,int node_id)
{
  return ccx_core.frd_check_node_exists(job_id,node_id);
}

std::vector<std::string> CalculiXFrdInterface::frd_get_result_block_types(int job_id)
{
  return ccx_core.frd_get_result_block_types(job_id);
}

std::vector<std::string> CalculiXFrdInterface::frd_get_result_block_components(int job_id, std::string result_block_type)
{
  return ccx_core.frd_get_result_block_components(job_id, result_block_type);
}

std::vector<int> CalculiXFrdInterface::frd_get_total_increments(int job_id)
{
  return ccx_core.frd_get_total_increments(job_id);
}

double CalculiXFrdInterface::frd_get_time_from_total_increment(int job_id, int total_increment)
{
  return ccx_core.frd_get_time_from_total_increment(job_id, total_increment);
}

std::vector<int> CalculiXFrdInterface::frd_get_node_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  return ccx_core.frd_get_node_ids_between_values(job_id,total_increment,result_block_type,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXFrdInterface::frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_core.frd_get_node_ids_smaller_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXFrdInterface::frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_core.frd_get_node_ids_greater_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXFrdInterface::frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  return ccx_core.frd_get_element_ids_between_values(job_id,total_increment,result_block_type,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXFrdInterface::frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_core.frd_get_element_ids_smaller_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXFrdInterface::frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  return ccx_core.frd_get_element_ids_greater_value(job_id,total_increment,result_block_type,result_block_component,value);
}

std::vector<int> CalculiXFrdInterface::frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit)
{
  return ccx_core.frd_get_element_ids_over_limit(job_id,total_increment,result_block_type,result_block_component,limit);
}

double CalculiXFrdInterface::frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component)
{
  return ccx_core.frd_get_node_value(job_id,node_id,total_increment,result_block_type,result_block_component);
}

std::vector<double> CalculiXFrdInterface::frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type)
{
  return ccx_core.frd_get_node_values(job_id,node_id,total_increment,result_block_type);
}
