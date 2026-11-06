#include "CoreResultsDatQuery.hpp"
#include "Core/results/CoreResults.hpp"
#include "CoreResultsDat.hpp"
#include <algorithm>

CoreResultsDatQuery::CoreResultsDatQuery(CoreResults& results)
  : results(results)
{
}

std::vector<std::string> CoreResultsDatQuery::dat_get_result_block_types(int job_id)
{
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return {};
  }

  return results.dat_data[dat_data_id].result_block_type;
}

std::vector<std::string> CoreResultsDatQuery::dat_get_result_block_set(int job_id)
{
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return {};
  }

  return results.dat_data[dat_data_id].result_block_set;
}

std::vector<std::string> CoreResultsDatQuery::dat_get_result_block_components(int job_id, std::string result_block_type)
{
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return {};
  }

  return results.dat_data[dat_data_id].get_result_block_components_from_result_block_type(result_block_type);
}

std::vector<double> CoreResultsDatQuery::dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set)
{
  std::vector<double> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      tmp.push_back(results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]);
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_result_block_nodes(int job_id, double time, std::string result_block_type, std::string result_block_set)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          //int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          tmp.push_back(c1);
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_result_block_elements(int job_id, double time, std::string result_block_type, std::string result_block_set)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          //int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          tmp.push_back(c1);
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((value >= lower_value)&&(value <= upper_value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value <= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value >= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((value >= lower_value)&&(value <= upper_value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value <= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value >= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsDatQuery::dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);
  int last_element_id = -1;

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];
      std::vector<double> values;

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];

          if (ii==0)
          {
            last_element_id = c1;
          }

          if (last_element_id!=c1)
          {
            double min_value=0;
            double max_value=0;
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }
              else
              {
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if limit is reached
            if (max_value - min_value > limit)
            {
              tmp.push_back(last_element_id);
            }
            //reset for next element
            last_element_id = c1;
            values.clear();
            values.push_back(value);
          }
          else if (results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size()==ii+1)
          {
            values.push_back(value);

            double min_value=0;
            double max_value=0;
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }
              else
              {
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if limit is reached
            if (max_value - min_value > limit)
            {
              tmp.push_back(c1);
            }
          }
          else
          {
            values.push_back(value);
          }
        }
      }
    }
  }
  return tmp;
}

double CoreResultsDatQuery::dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  double tmp = 0;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if ((results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0]==node_id)&&(results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1)) //check if nodal values and if node id fits
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double tmp = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          return tmp;
        }
      }
    }
  }
  return tmp;
}

std::vector<double> CoreResultsDatQuery::dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set)
{
  std::vector<double> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if ((results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0]==node_id)&&(results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1)) //check if nodal values and if node id fits
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          for (size_t iii = 0; iii < results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id].size(); iii++)
          {
            double value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][iii];
            tmp.push_back(value);
          }
          return tmp;
        }
      }
    }
  }
  return tmp;
}

std::vector<double> CoreResultsDatQuery::dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  std::vector<double> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results.dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];
      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        const auto& record = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii];
        if (record[0] == element_id && (record[2] == 2 || record[2] == 3))
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          tmp.push_back(value);
        }
      }
    }
  }
  return tmp;
}

std::vector<std::vector<double>> CoreResultsDatQuery::dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set)
{
  std::vector<std::vector<double>> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results.dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results.dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results.dat_data[dat_data_id].total_times[results.dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results.dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results.dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results.dat_data[dat_data_id].result_blocks[i][4];

      for (size_t ii = 0; ii < results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        const auto& record = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii];
        if (record[0] == element_id && (record[2] == 2 || record[2] == 3)) //check if element values and if node id fits
        {
          int c1 = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results.dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          std::vector<double> values;
          for (size_t iii = 0; iii < results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id].size(); iii++)
          {
            double value = results.dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][iii];
            values.push_back(value);
          }
          tmp.push_back(values);
        }
      }
    }
  }
  return tmp;
}

std::vector<std::vector<std::vector<double>>> CoreResultsDatQuery::dat_get_buckle(int job_id)
{
  std::vector<std::vector<std::vector<double>>> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  return results.dat_data[dat_data_id].buckle_data;
}

std::vector<std::string> CoreResultsDatQuery::dat_get_section_set(int job_id)
{
  std::vector<std::string> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  return results.dat_data[dat_data_id].result_section_set;
}

std::vector<std::vector<double>> CoreResultsDatQuery::dat_get_section_data(int job_id, std::string section_set)
{
  std::vector<std::vector<double>> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.dat_data[dat_data_id].result_section.size(); i++)
  {
    if (results.dat_data[dat_data_id].result_section_set[results.dat_data[dat_data_id].result_section[i][0]]==section_set)
    {
      tmp.push_back(results.dat_data[dat_data_id].result_section_data[results.dat_data[dat_data_id].result_section[i][1]]);
    }
  }

  return tmp;
}

std::vector<std::string> CoreResultsDatQuery::dat_get_section_label(int job_id)
{
  std::vector<std::string> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int dat_data_id = results.get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  return results.dat_data[dat_data_id].result_section_label;
}


std::vector<DatResultTable> CoreResultsDatQuery::dat_get_table_metadata(int job_id)
{
  const int id = results.get_dat_data_id_from_job_id(job_id);
  if (id < 0 || static_cast<std::size_t>(id) >= results.dat_data.size())
  {
    return {};
  }
  std::vector<DatResultTable> metadata;
  for (const auto& table : results.dat_data[id].result_tables)
  {
    metadata.push_back({table.key, table.name, table.set_name, table.slave_set, table.master_set,
      table.step, table.increment, table.time, table.source_line, table.columns, {}});
  }
  return metadata;
}

std::vector<std::vector<double>> CoreResultsDatQuery::dat_get_table_rows(int job_id, int table_id, std::size_t offset, std::size_t count)
{
  const int id = results.get_dat_data_id_from_job_id(job_id);
  if (id < 0 || static_cast<std::size_t>(id) >= results.dat_data.size() || table_id < 0 ||
    static_cast<std::size_t>(table_id) >= results.dat_data[id].result_tables.size())
  {
    return {};
  }
  const auto& rows = results.dat_data[id].result_tables[table_id].rows;
  if (offset >= rows.size())
  {
    return {};
  }
  const auto end = offset + std::min(count, rows.size() - offset);
  return {rows.begin() + offset, rows.begin() + end};
}

std::size_t CoreResultsDatQuery::dat_get_table_row_count(int job_id, int table_id)
{
  const int id = results.get_dat_data_id_from_job_id(job_id);
  if (id < 0 || static_cast<std::size_t>(id) >= results.dat_data.size() || table_id < 0 ||
    static_cast<std::size_t>(table_id) >= results.dat_data[id].result_tables.size())
  {
    return 0;
  }
  return results.dat_data[id].result_tables[table_id].rows.size();
}
