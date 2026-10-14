#include "CoreResultsProject.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CoreResultsFrd.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "AppUtil.hpp"
#include "MeshExportInterface.hpp"

#include <cmath>
#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <sstream>

CoreResultsProject::CoreResultsProject()
{}

CoreResultsProject::~CoreResultsProject()
{}

bool CoreResultsProject::init(CoreResultsFrd* frd)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    ccx_iface = new CalculiXCoreInterface();

    this->frd = frd;

    progressbar = CubitInterface::app_util().get()->progress_tool();;
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

  if (data_id == -1) // step or increment not found
  {
    log = "Requested data set not found.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  progressbar->start(0,100,"Projecting Displacements");
  auto t_start = std::chrono::high_resolution_clock::now();

  log = "Setting node constraint off.\n";
  PRINT_INFO("%s", log.c_str());

  if (!ccx_iface->silent_cmd("set developer on")){return false;}
  if (!ccx_iface->silent_cmd("set node constraint off")){return false;}
  
  cmd = "graphics off";
  ccx_iface->silent_cmd(cmd.c_str());

  //block core and update
  ccx_iface->set_block_core_update(true);
  ccx_iface->set_block_gui_update(true);
    
  for (size_t i = 0; i < frd->result_block_node_data[data_id].size(); i++)
  {
    //node 1 move x 0.1 y 0.2 z 0.3
    cmd = "node " + std::to_string(frd->result_block_node_data[data_id][i][0])
    + " move"
    + " x " + ccx_iface->to_string_scientific(scale*frd->result_block_data[data_id][frd->result_block_node_data[data_id][i][1]][0])
    + " y " + ccx_iface->to_string_scientific(scale*frd->result_block_data[data_id][frd->result_block_node_data[data_id][i][1]][1])
    + " z " + ccx_iface->to_string_scientific(scale*frd->result_block_data[data_id][frd->result_block_node_data[data_id][i][1]][2])
    ;
    
    ccx_iface->silent_cmd(cmd.c_str());

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
  ccx_iface->silent_cmd(cmd.c_str());
  
  //block core and update
  ccx_iface->set_block_core_update(false);
  ccx_iface->set_block_gui_update(false);
  
  if (!ccx_iface->silent_cmd("set developer off")){return false;}

  return true;
}

bool CoreResultsProject::project_topology(int topology_id,int iteration_id,double threshold,bool preview)
{
  if (iteration_id < -1 ||
      !std::isfinite(threshold) ||
      threshold < 0.0 || threshold > 1.0)
  {
    PRINT_ERROR("Invalid topology iteration or threshold.\n");
    return false;
  }

  const std::string state = ccx_iface->get_topology_run_state(topology_id);

  if (state.empty())
  {
    return false;
  }

  if (state == "running")
  {
    PRINT_ERROR("Cannot project a running topology case.\n");
    return false;
  }

  const auto rows = ccx_iface->get_topology_iteration_element_data(topology_id, iteration_id);

  if (rows.empty())
  {
    return false;
  }

  std::map<std::string, std::vector<int>> removal;
  std::set<int> seen;
  size_t removal_count = 0;

  // Validate the entire design before changing the mesh.
  for (const auto& row : rows)
  {
    if (row.size() != 4 ||
        !std::isfinite(row[0]) ||
        row[0] < 1.0 ||
        row[0] > std::numeric_limits<int>::max() ||
        std::floor(row[0]) != row[0] ||
        !std::isfinite(row[1]) ||
        row[1] < 0.0 || row[1] > 1.0 ||
        (row[3] != 0.0 && row[3] != 1.0))
    {
      PRINT_ERROR("Invalid topology element data.\n");
      return false;
    }

    const int global_id = static_cast<int>(row[0]);

    if (!seen.insert(global_id).second)
    {
      PRINT_ERROR("Duplicate topology element %d.\n", global_id);
      return false;
    }

    if (!CubitInterface::get_element_exists(global_id))
    {
      PRINT_ERROR("Topology element %d no longer exists in Cubit.\n", global_id);
      return false;
    }

    const std::string element_type = CubitInterface::get_element_type(global_id);

    std::string entity = element_type;

    if (entity != "hex" && entity != "tet" && entity != "wedge")
    {
      entity = ccx_iface->get_cubit_element_type_entity(element_type);
    }

    const int local_id = CubitInterface::get_element_type_id(global_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
    {
      PRINT_ERROR("Invalid topology volume element %d.\n", global_id);
      return false;
    }

    const bool preserved = row[3] == 1.0;

    if (!preserved && row[1] < threshold)
    {
      removal[entity].push_back(local_id);
      ++removal_count;
    }
  }

  if (preview)
  {
    return ccx_iface->draw_topology(topology_id, iteration_id, threshold);
  }

  if (removal_count == 0)
  {
    PRINT_INFO("Topology %d: no elements are below threshold %.6g.\n", topology_id, threshold);
    return true;
  }

  // Prepare every command before starting deletion.
  std::vector<std::pair<std::string, size_t>> commands;
  const size_t batch_size = 200;

  for (const auto& group : removal)
  {
    for (size_t begin = 0; begin < group.second.size();
         begin += batch_size)
    {
      const size_t end =
          std::min(begin + batch_size, group.second.size());

      std::ostringstream command;
      command << "delete " << group.first;

      for (size_t index = begin; index < end; ++index)
      {
        command << " " << group.second[index];
      }

      // Delete volume elements while retaining their child mesh.
      if (group.first != "wedge")
      {
        command << " no_propagate";
      }
      commands.emplace_back(command.str(), end - begin);
    }
  }

  const bool developer_was_enabled = CubitInterface::developer_commands_are_enabled();

  if (!developer_was_enabled && !ccx_iface->silent_cmd("set developer commands on"))
  {
    PRINT_ERROR("Unable to enable element deletion.\n");
    return false;
  }

  // Restore the previous developer setting on every return path.
  struct DeveloperGuard
  {
    CalculiXCoreInterface& iface;
    bool restore;

    ~DeveloperGuard()
    {
      if (restore)
      {
        if (!iface.silent_cmd("set developer commands off"))
        {
          PRINT_ERROR("Unable to restore developer setting.\n");
        }
      }
    }
  } developer_guard{*ccx_iface, !developer_was_enabled};

  size_t deleted_count = 0;

  for (const auto& command : commands)
  {
    if (!ccx_iface->silent_cmd(command.first))
    {
      PRINT_ERROR(
          "Topology deletion failed after %zu elements were "
          "deleted in completed batches; the failed batch "
          "may also have changed the mesh.\n",
          deleted_count);
      return false;
    }

    deleted_count += command.second;
  }

  PRINT_INFO(
      "Topology %d: deleted %zu elements below threshold %.6g; "
      "preserved elements were excluded.\n",
      topology_id, deleted_count, threshold);

  if (!ccx_iface->silent_cmd("display"))
  {
    PRINT_ERROR("Elements were deleted, but display refresh failed.\n");
    return false;
  }

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
  //check node count for vertex in reference point
  //check node count for curve in trajectory
  int free_node_count = 0;
  free_node_count = int(CubitInterface::get_list_of_free_ref_entities("vertex").size());
  std::vector<int> trajectory_ids = ccx_iface->get_loadstrajectory_ids();
  std::vector<int> tmp_node_ids;
  for (size_t i = 0; i < trajectory_ids.size(); i++)
  {
    std::vector<int> tmp_hf_node_ids = ccx_iface->loadstrajectory_heatflux_get_node_ids(trajectory_ids[i]);
    std::vector<int> tmp_bfs_node_ids = ccx_iface->loadstrajectory_bodyheatfluxsphere_get_node_ids(trajectory_ids[i]);
    
    for (size_t ii = 0; ii < tmp_hf_node_ids.size(); ii++)
    {
      tmp_node_ids.push_back(tmp_hf_node_ids[ii]);
    }
    for (size_t ii = 0; ii < tmp_bfs_node_ids.size(); ii++)
    {
      tmp_node_ids.push_back(tmp_bfs_node_ids[ii]);
    }
  }

  std::sort(tmp_node_ids.begin(), tmp_node_ids.end());
  int last_id = 0;
  for (size_t i = 0; i < tmp_node_ids.size(); i++)
  {
    if (last_id != tmp_node_ids[i])
    {
      free_node_count = free_node_count + 1;
      last_id = tmp_node_ids[i];
    }
  }
  
  if (CubitInterface::get_node_count()!=frd->nodes.size() + free_node_count)
  {
    log = "Linking Failed! Wrong number of Nodes.\n";
    log.append("Cubit Node Count:" + std::to_string(CubitInterface::get_node_count()) + "\n");
    log.append("FRD   Node Count:" + std::to_string(frd->nodes.size()) + "\n");
    log.append("Free  Node Count:" + std::to_string(free_node_count) + "\n");
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  //check element count for vertex in reference point
  //check element count for curve in trajectory
  int free_element_count = 0;
  free_element_count = int(CubitInterface::get_list_of_free_ref_entities("vertex").size());
  trajectory_ids = ccx_iface->get_loadstrajectory_ids();
  for (size_t i = 0; i < trajectory_ids.size(); i++)
  {
    free_element_count = free_element_count + int(ccx_iface->loadstrajectory_heatflux_get_edge_ids(trajectory_ids[i]).size());
    free_element_count = free_element_count + int(ccx_iface->loadstrajectory_bodyheatfluxsphere_get_edge_ids(trajectory_ids[i]).size());
  }
  if (CubitInterface::get_element_count()!=frd->elements.size()+free_element_count)
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
      data_id = int(i);
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
      data_id = int(i);
    }
  }
  return data_id;
}