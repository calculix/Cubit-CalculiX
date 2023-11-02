#include "CoreResultsProject.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CoreResultsFrd.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "MeshExportInterface.hpp"

#include <cmath>
#include <bits/stdc++.h> 

CoreResultsProject::CoreResultsProject()
{}

CoreResultsProject::~CoreResultsProject()
{
}

bool CoreResultsProject::init(CoreResultsFrd* frd)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

    this->frd = frd;

    progressbar = new ProgressTool();
    // otherwise cubitinterface gets false element count!
    me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
    me_iface->initialize_export();

    is_initialized = true;  
    return true;
  }
}
bool CoreResultsProject::reset()
{
  clear();
  init(nullptr);
  return true;
}
bool CoreResultsProject::clear()
{
  return true;
}
bool CoreResultsProject::project(int step, int totalincrement,double scale)
{
  std::string log;
  std::string cmd;
  int data_id;

  if (!checkLinkPossible()){return false;}


  if (step!=-1)
  {
    data_id = get_result_blocks_data_id_step(step);
  }else{
    data_id = get_result_blocks_data_id_totalincrement(totalincrement);
  }

  //this->stopwatch("start");

  progressbar->start(0,100,"Projecting Displacements");
  auto t_start = std::chrono::high_resolution_clock::now();

  log = "Setting node constraint off.\n";
  PRINT_INFO("%s", log.c_str());

  if (!CubitInterface::silent_cmd_without_running_journal_lines("set node constraint off")){return false;}
  
  cmd = "graphics off";
  CubitInterface::silent_cmd_without_running_journal_lines(cmd.c_str());

  for (size_t i = 0; i < frd->result_block_node_data[data_id].size(); i++)
  {
    //node 1 move x 0.1 y 0.2 z 0.3
    cmd = "node " + std::to_string(frd->result_block_node_data[data_id][i][0])
    + " move"
    + " x " + ccx_iface->to_string_scientific(scale*frd->result_block_data[data_id][frd->result_block_node_data[data_id][i][1]][0])
    + " y " + ccx_iface->to_string_scientific(scale*frd->result_block_data[data_id][frd->result_block_node_data[data_id][i][1]][1])
    + " z " + ccx_iface->to_string_scientific(scale*frd->result_block_data[data_id][frd->result_block_node_data[data_id][i][1]][2])
    ;
    
    CubitInterface::silent_cmd_without_running_journal_lines(cmd.c_str());

    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    if (duration > 500)
    {
      progressbar->percent(double(i)/double(frd->result_block_node_data[data_id].size()));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();
  
  cmd = "graphics on";
  CubitInterface::silent_cmd_without_running_journal_lines(cmd.c_str());

  //this->stopwatch("stop");

  return true;
}

bool CoreResultsProject::checkLinkPossible()
{
  std::string log;

  if (frd->nodes.size()==0)
  {
    return false;
  }
  
  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    if (!CubitInterface::get_node_exists(frd->nodes[i][0]))
    {
      log = "Linking Failed! Node " + std::to_string(frd->nodes[i][0]) + " doesn't exist in Cubit.\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
  }
  for (size_t i = 0; i < frd->elements.size(); i++)
  {
    if (!CubitInterface::get_element_exists(frd->elements[i][0]))
    { 
      log = "Linking Failed! Element " + std::to_string(frd->elements[i][0]) + " doesn't exist in Cubit.\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
  }
  if (CubitInterface::get_node_count()!=frd->nodes.size()+CubitInterface::get_list_of_free_ref_entities("vertex").size())
  {
    log = "Linking Failed! Wrong number of Nodes.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (CubitInterface::get_element_count()!=frd->elements.size()+CubitInterface::get_list_of_free_ref_entities("vertex").size())
  {
    log = "Linking Failed! Wrong number of Elements.\n";
    //log.append("Cubit " + std::to_string(CubitInterface::get_element_count()) + " FRD " + std::to_string(frd->elements.size()+CubitInterface::get_list_of_free_ref_entities("vertex").size()) +  "\n");
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  return true;
}

int CoreResultsProject::get_result_blocks_data_id_step(int step)
{
  int data_id = -1;
  
  for (size_t i = 0; i < frd->result_blocks.size(); i++)
  {
    if ((step == frd->result_blocks[i][1])&&(frd->result_block_type[frd->result_blocks[i][5]]=="DISP"))
    {
      data_id = i;
    }
  }
  return data_id;
}

int CoreResultsProject::get_result_blocks_data_id_totalincrement(int totalincrement)
{
  int data_id = -1;
  
  for (size_t i = 0; i < frd->result_blocks.size(); i++)
  {
    if ((totalincrement == frd->result_blocks[i][3])&&(frd->result_block_type[frd->result_blocks[i][5]]=="DISP"))
    {
      data_id = i;
    }
  }
  return data_id;
}

bool CoreResultsProject::stopwatch(std::string label)
{
  const auto t_runtime_end = std::chrono::high_resolution_clock::now();
  int t_runtime_duration = std::chrono::duration<double, std::milli>(t_runtime_end - t_runtime_start).count();
  int t_runtime_round_duration = std::chrono::duration<double, std::milli>(t_runtime_end - t_runtime_last).count();
  
  t_runtime_last = std::chrono::high_resolution_clock::now();
  
  std::string log;
  log =  label + " " + std::to_string(t_runtime_duration) + " " + std::to_string(t_runtime_round_duration) + "\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}
