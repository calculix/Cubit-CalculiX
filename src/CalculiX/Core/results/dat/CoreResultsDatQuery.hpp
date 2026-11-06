#ifndef CORERESULTSDATQUERY_HPP
#define CORERESULTSDATQUERY_HPP

#include <string>
#include <vector>
#include <cstddef>
#include "Core/results/dat/CoreResultsDatStructs.hpp"

class CoreResults;

class CoreResultsDatQuery
{
public:
  explicit CoreResultsDatQuery(CoreResults& results);

  std::vector<std::string> dat_get_result_block_types(int job_id); // returns a list of all result block types
  std::vector<std::string> dat_get_result_block_set(int job_id); // returns a list of all result block set
  std::vector<std::string> dat_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type and set
  std::vector<double> dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set); // returns a list of all result block times for a block type and set
  std::vector<int> dat_get_result_block_nodes(int job_id, double time, std::string result_block_type, std::string result_block_set); // returns a list of all result block nodes for a block type and set
  std::vector<int> dat_get_result_block_elements(int job_id, double time, std::string result_block_type, std::string result_block_set); // returns a list of all result block elements for a block type and set
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
  std::vector<std::vector<std::vector<double>>> dat_get_buckle(int job_id); // returns the buckling data for a job
  std::vector<std::string> dat_get_section_set(int job_id); // returns the section set for a job
  std::vector<std::vector<double>> dat_get_section_data(int job_id, std::string section_set); // returns the section set data for a job
  std::vector<std::string> dat_get_section_label(int job_id); // returns the section labels for a job

  // Metadata excludes rows. Read table values in bounded pages through this API.
  std::vector<DatResultTable> dat_get_table_metadata(int job_id);
  std::size_t dat_get_table_row_count(int job_id, int table_id);
  std::vector<std::vector<double>> dat_get_table_rows(int job_id, int table_id, std::size_t offset, std::size_t count);


private:
  CoreResults& results;
};

#endif // CORERESULTSDATQUERY_HPP
