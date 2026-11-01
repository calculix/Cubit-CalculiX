#include "CoreResultsCsv.hpp"
#include "Core/export/CoreCsvWriter.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

CoreResultsCsv::CoreResultsCsv(CoreResults& results)
  : frd_query(results), dat_query(results)
{
}

bool CoreResultsCsv::result_csv_job_frd(int job_id,std::string block_type, std::string block_component, std::string increment,int node_id,int block_id,int nodeset_id,int sideset_id, bool overwrite, std::string save_filepath)
{
  std::string log;
  //log = "exporting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if(job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (block_type=="")
  {
    log = "Can't export results -> no result block set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if(block_component=="")
  {
    log = "Can't export results -> no result component set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  if (increment=="")
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  std::vector<int> nodes;
  std::vector<int> frd_nodes = frd_query.frd_get_nodes(job_id);

  // check if filter was chosen
  if (block_id!=-1)
  {
    std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in block " + std::to_string(block_id));;
    for (size_t i = 0; i < node_ids.size(); i++)
    {
      if (frd_query.frd_check_node_exists(job_id, node_ids[i]))
      {
        nodes.push_back(node_ids[i]);
      }
    }
  }
  if (nodeset_id!=-1)
  {
    std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in nodeset " + std::to_string(nodeset_id));;
    for (size_t i = 0; i < node_ids.size(); i++)
    {
      if (frd_query.frd_check_node_exists(job_id, node_ids[i]))
      {
        nodes.push_back(node_ids[i]);
      }
    }
  }
  if (sideset_id!=-1)
  {
    std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in sideset " + std::to_string(sideset_id));;
    for (size_t i = 0; i < node_ids.size(); i++)
    {
      if (frd_query.frd_check_node_exists(job_id, node_ids[i]))
      {
        nodes.push_back(node_ids[i]);
      }
    }
  }

  if (node_id > 0)
  {
    if (frd_query.frd_check_node_exists(job_id, node_id))
    {
        nodes.push_back(node_id);
    }
    else
    {
      log = "Can't find node id " + std::to_string(node_id) + " in frd data -> reference points for example are not written into frd \n";
      PRINT_INFO("%s", log.c_str());
    }
  }

  if (nodes.size()==0) // this means no filter for sets was applied
  {
    nodes = frd_nodes;
  }

  //prepare components
  std::vector<std::string> components;
  if (block_component=="all")
  {
    components = frd_query.frd_get_result_block_components(job_id, block_type);
  }
  else
  {
    components.push_back(block_component);
  }

  //prepare increments
  std::vector<int> increments;
  if (increment=="all")
  {
    increments = frd_query.frd_get_total_increments(job_id);
  }
  else
  {
    increments.push_back(std::stoi(increment));
  }

  //get header
  std::vector<std::string> header;
  header.push_back("Node ID");
  header.push_back("Increment");
  header.push_back("Time");
  for (size_t ii = 0; ii < components.size(); ii++)
  {
    header.push_back(components[ii]);
  }

  //get results
  std::vector<std::vector<double>> results;
  if ((nodes.size()>0)&&(components.size()>0)&&(increments.size()>0)) //check if data can be queried
  {
    for (size_t i = 0; i < nodes.size(); i++)
    {
      for (size_t ii = 0; ii < increments.size(); ii++)
      {
        std::vector<double> tmp_result;
        double increment_time = frd_query.frd_get_time_from_total_increment(job_id, increments[ii]);
        tmp_result.push_back(double(nodes[i]));
        tmp_result.push_back(double(increments[ii]));
        tmp_result.push_back(increment_time);
        for (size_t iii = 0; iii < components.size(); iii++)
        {
          double node_result = frd_query.frd_get_node_value(job_id, nodes[i] , increments[ii], block_type, components[iii]);
          tmp_result.push_back(node_result);
        }
        results.push_back(tmp_result);
      }
    }
  }
  //write csv
  return CoreCsvWriter::write(save_filepath, header, results, overwrite);
}

bool CoreResultsCsv::result_csv_job_dat(int job_id,std::string block_type,std::string block_set, std::string block_component, std::string time,int node_id,int element_id,bool overwrite, std::string save_filepath)
{
  std::string log;
  //log = "exporting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if(job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (block_set=="")
  {
    log = "Can't export results -> no result_set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (block_type=="")
  {
    log = "Can't export results -> no result block set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if(block_component=="")
  {
    log = "Can't export results -> no result component set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (time=="")
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if ((node_id>0)&&(element_id>0))
  {
    log = "Can't export results -> Can't filter for Node ID and Element ID at the same time! \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  //prepare components
  std::vector<std::string> components;
  if (block_component=="all")
  {
    components = dat_query.dat_get_result_block_components(job_id, block_type);
  }
  else
  {
    components.push_back(block_component);
  }

  //prepare times
  std::vector<double> times;
  if (time=="all")
  {
    times = dat_query.dat_get_result_block_times(job_id,block_type,block_set);
  }
  else
  {
    times.push_back(std::stod(time));
  }

  if (times.empty())
  {
    PRINT_ERROR("Can't export results -> no matching result times\n");
    return false;
  }

  //get nodes and elements
  std::vector<int> nodes;
  std::vector<int> elements;

  if (node_id>0)
  {
    nodes.push_back(node_id);
  }
  else
  {
    nodes = dat_query.dat_get_result_block_nodes(job_id,times[0],block_type,block_set);
  }
  if (element_id>0)
  {
    elements.push_back(element_id);
  }
  else
  {
    elements = dat_query.dat_get_result_block_elements(job_id,times[0],block_type,block_set);
  }

  if ((nodes.size()>0)&&(elements.size()>0))
  {
    log = "Can't export results -> Can't request for Nodes and Element Data at the same time! \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  //get results
  std::vector<std::vector<double>> node_results;
  std::vector<std::vector<double>> element_results;

  if ((nodes.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    int ic = 0;
    for (size_t i = 0; i < nodes.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      {
        std::vector<double> tmp_result;
        tmp_result.push_back(double(nodes[i]));
        tmp_result.push_back(double(times[ii]));
        for (size_t iii = 0; iii < components.size(); iii++)
        {
          double node_result = dat_query.dat_get_node_value(job_id, nodes[i],times[ii],block_type,block_set, components[iii]);
          tmp_result.push_back(node_result);
        }
        node_results.push_back(tmp_result);
      }
    }
  }

  if ((elements.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    int ic = 0;
    for (size_t i = 0; i < elements.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      {
        std::vector<std::vector<double>> tmp_result;
        for (size_t iii = 0; iii < components.size(); iii++)
        {
          std::vector<double> element_result = dat_query.dat_get_element_values_for_component(job_id, elements[i],times[ii],block_type,block_set, components[iii]);
          if (iii == 0)
          {
            for (size_t iv = 0; iv < element_result.size(); iv++)
            {
              std::vector<double> tmp;
              tmp.push_back(double(elements[i]));
              tmp.push_back(double(times[ii]));
              tmp.push_back(element_result[iv]);
              tmp_result.push_back(tmp);
            }
          }
          else
          {
            for (size_t iv = 0; iv < element_result.size(); iv++)
            {
              tmp_result[iv].push_back(element_result[iv]);
            }
          }
        }
        for (size_t iii = 0; iii < tmp_result.size(); iii++)
        {
          element_results.push_back(tmp_result[iii]);
        }
      }
    }
  }
  std::vector<std::string> header;

  if (!node_results.empty())
  {
    header = {"Node ID", "Time"};
    header.insert(header.end(), components.begin(), components.end());
    return CoreCsvWriter::write(save_filepath, header, node_results, overwrite);
  }

  if (!element_results.empty())
  {
    header = {"Element ID", "Time"};
    header.insert(header.end(), components.begin(), components.end());
    return CoreCsvWriter::write(save_filepath, header, element_results, overwrite);
  }

  return false;
}

bool CoreResultsCsv::result_csv_job_dat_section(int job_id, std::string result_set, std::string time, bool overwrite, std::string save_filepath)
{
  std::string log;

  if(job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (result_set=="")
  {
    log = "Can't export results -> no result_set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (time=="")
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  // get results
  std::vector<std::vector<double>> section_data = dat_query.dat_get_section_data(job_id,result_set);

  //prepare components
  std::vector<std::string> components;
  components = dat_query.dat_get_section_label(job_id);

  //prepare times
  std::vector<double> times;
  if (time=="all")
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      times.push_back(section_data[i][0]);
    }
  }
  else
  {
    times.push_back(std::stod(time));
  }

  //get results
  std::vector<std::vector<double>> section_results;

  if ((section_data.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    bool bool_time = false;
    for (size_t i = 0; i < section_data.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      {
        if (section_data[i][0]==times[ii])
        {
          bool_time = true;
          break;
        }
      }

      if (bool_time)// check if items are in range
      {
        section_results.push_back(section_data[i]);
      }
      bool_time = false;
    }
  }

  if (section_results.empty())
  {
    return false;
  }

  return CoreCsvWriter::write(save_filepath, components, section_results, overwrite);
}

