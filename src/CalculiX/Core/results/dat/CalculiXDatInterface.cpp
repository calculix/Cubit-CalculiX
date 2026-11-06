#include "CalculiXDatInterface.hpp"
#include "CoreResultsDatQuery.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<std::string> CalculiXDatInterface::dat_get_result_block_types(int job_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_result_block_types(job_id);
}

std::vector<std::string> CalculiXDatInterface::dat_get_result_block_set(int job_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_result_block_set(job_id);
}

std::vector<std::string> CalculiXDatInterface::dat_get_result_block_components(int job_id, std::string result_block_type)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_result_block_components(job_id, result_block_type);
}

std::vector<double> CalculiXDatInterface::dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_result_block_times(job_id, result_block_type, result_block_set);
}

std::vector<int> CalculiXDatInterface::dat_get_result_block_nodes(int job_id, double time, std::string result_block_type, std::string result_block_set)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_result_block_nodes(job_id, time, result_block_type, result_block_set);
}

std::vector<int> CalculiXDatInterface::dat_get_result_block_elements(int job_id, double time, std::string result_block_type, std::string result_block_set)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_result_block_elements(job_id, time, result_block_type, result_block_set);
}

std::vector<int> CalculiXDatInterface::dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_node_ids_between_values(job_id,time,result_block_type,result_block_set,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXDatInterface::dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_node_ids_smaller_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXDatInterface::dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_node_ids_greater_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXDatInterface::dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_element_ids_between_values(job_id,time,result_block_type,result_block_set,result_block_component,lower_value,upper_value);
}

std::vector<int> CalculiXDatInterface::dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_element_ids_smaller_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXDatInterface::dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_element_ids_greater_value(job_id,time,result_block_type,result_block_set,result_block_component,value);
}

std::vector<int> CalculiXDatInterface::dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_element_ids_over_limit(job_id,time,result_block_type,result_block_set,result_block_component,limit);
}

double CalculiXDatInterface::dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_node_value(job_id,node_id,time,result_block_type,result_block_set,result_block_component);
}

std::vector<double> CalculiXDatInterface::dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_node_values(job_id,node_id,time,result_block_type,result_block_set);
}

std::vector<double> CalculiXDatInterface::dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_element_values_for_component(job_id,element_id,time,result_block_type,result_block_set,result_block_component);
}

std::vector<std::vector<double>> CalculiXDatInterface::dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_element_values(job_id,element_id,time,result_block_type,result_block_set);
}

std::vector<std::vector<std::vector<double>>> CalculiXDatInterface::dat_get_buckle(int job_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_buckle(job_id);
}

std::vector<std::string> CalculiXDatInterface::dat_get_section_set(int job_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_section_set(job_id);
}

std::vector<std::vector<double>> CalculiXDatInterface::dat_get_section_data(int job_id, std::string section_set)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_section_data(job_id, section_set);
}

std::vector<std::string> CalculiXDatInterface::dat_get_section_label(int job_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_section_label(job_id);
}

std::vector<DatResultTable> CalculiXDatInterface::dat_get_table_metadata(int job_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_table_metadata(job_id);
}

std::vector<std::vector<double>> CalculiXDatInterface::dat_get_table_rows(int job_id, int table_id, std::size_t offset, std::size_t count)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_table_rows(job_id, table_id, offset, count);
}

std::size_t CalculiXDatInterface::dat_get_table_row_count(int job_id, int table_id)
{
  return CoreResultsDatQuery(*ccx_core.results).dat_get_table_row_count(job_id, table_id);
}
