#include "CoreResultsFrdQuery.hpp"
#include "Core/results/CoreResults.hpp"
#include "CoreResultsFrd.hpp"

#include <algorithm>

CoreResultsFrdQuery::CoreResultsFrdQuery(CoreResults& results)
  : results(results)
{
}

std::vector<int> CoreResultsFrdQuery::frd_get_nodes(int job_id)
{
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return {};
  }

  return results.frd_data[frd_data_id].sorted_node_ids;
}

bool CoreResultsFrdQuery::frd_check_node_exists(int job_id,int node_id)
{
  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return false;
  }

  const auto& node_ids = results.frd_data[frd_data_id].sorted_node_ids;
  return std::binary_search(node_ids.begin(), node_ids.end(), node_id);
}

std::vector<std::string> CoreResultsFrdQuery::frd_get_result_block_types(int job_id)
{
  std::vector<std::string> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results.frd_data[frd_data_id].result_block_type;

  return tmp;
}

std::vector<std::string> CoreResultsFrdQuery::frd_get_result_block_components(int job_id, std::string result_block_type)
{
  std::vector<std::string> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results.frd_data[frd_data_id].get_result_block_components_from_result_block_type(result_block_type);

  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_total_increments(int job_id)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    if (i==0)
    {
      tmp.push_back(results.frd_data[frd_data_id].result_blocks[i][3]);
    }
    else
    {
      // don't use double entries
      if (tmp[tmp.size()-1]!=results.frd_data[frd_data_id].result_blocks[i][3])
      {
        tmp.push_back(results.frd_data[frd_data_id].result_blocks[i][3]);
      }
    }
  }

  return tmp;
}

double CoreResultsFrdQuery::frd_get_time_from_total_increment(int job_id, int total_increment)
{
  double tmp = -1;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    if (total_increment==results.frd_data[frd_data_id].result_blocks[i][3])
    {
      tmp = results.frd_data[frd_data_id].total_times[results.frd_data[frd_data_id].result_blocks[i][4]];
      return tmp;
    }
  }

  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_node_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int node_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double value = results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if ((value >= lower_value)&&(value <= upper_value))
            {
              tmp.push_back(node_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int node_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double node_value = results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if (node_value <= value)
            {
              tmp.push_back(node_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int node_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double node_value = results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if (node_value >= value)
            {
              tmp.push_back(node_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results.frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results.frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results.frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
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
            // check if values are in range
            if ((min_value >= lower_value)&&(max_value <= upper_value))
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results.frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results.frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results.frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
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
            // check if values are in range
            if (max_value <= value)
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results.frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results.frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results.frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
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
            // check if values are in range
            if (min_value >= value)
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CoreResultsFrdQuery::frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit)
{
  std::vector<int> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results.frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results.frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results.frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results.frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results.frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
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
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

double CoreResultsFrdQuery::frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component)
{
  double tmp = 0;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results.frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results.frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results.frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int result_node_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double value = results.frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if (result_node_id == node_id)
            {
              return value;
            }
          }
        }
      }
    }
  }

  return tmp;
}

std::vector<double> CoreResultsFrdQuery::frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type)
{
  std::vector<double> tmp;

  int results_data_id = results.get_results_data_id_from_job_id(job_id);
  int frd_data_id = results.get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results.frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results.frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results.frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results.frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        // loop over all nodes in result block
        for (size_t ii = 0; ii < results.frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
        {
          int result_node_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][0];
          int node_data_id = results.frd_data[frd_data_id].result_block_node_data[i][ii][1];
          if (result_node_id == node_id)
          {
            return results.frd_data[frd_data_id].result_block_data[i][node_data_id];
          }
        }
      }
    }
  }

  return tmp;
}

