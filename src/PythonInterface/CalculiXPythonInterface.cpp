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
  log.append("write some help here....\n");
  log.append("write some help here....\n");
  log.append("write some help here....\n");
  log.append("write some help here....\n");
  PRINT_INFO("%s", log.c_str());
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

std::vector<int> CalculiXPythonInterface::frd_get_node_ids_between_limits(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_limit,double upper_limit)
{
  return ccx_iface->frd_get_node_ids_between_limits(job_id,total_increment,result_block_type,result_block_component,lower_limit,upper_limit);
}

double CalculiXPythonInterface::frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component)
{
  return ccx_iface->frd_get_node_value(job_id,node_id,total_increment,result_block_type,result_block_component);
}

std::vector<double> CalculiXPythonInterface::frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type)
{
  return ccx_iface->frd_get_node_values(job_id,node_id,total_increment,result_block_type);
}