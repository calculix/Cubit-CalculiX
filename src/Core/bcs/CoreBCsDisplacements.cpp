#include "Core/bcs/CoreBCsDisplacements.hpp"
#include <string>
#include <vector>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
CoreBCsDisplacements::CoreBCsDisplacements()
{}

CoreBCsDisplacements::~CoreBCsDisplacements()
{}

bool CoreBCsDisplacements::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreBCsDisplacements::update()
{
  return true;
}

bool CoreBCsDisplacements::reset()
{
  bcs_data.clear();
  time_delay_data.clear();
  displacement_data.clear();
  mode_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreBCsDisplacements::check_initialized()
{
  return is_initialized;
}

bool CoreBCsDisplacements::create_bc(std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3)
{
  // options: op mode, amplitude, time delay, nodeset, name
  // options2: prescribed values, options3: modes for dofs 1..6
  if (options.size() != 5 || options2.size() != 6 || options3.size() != 6)
  {
    return false;
  }

  for (size_t i = 0; i < 6; i++)
  {
    if (options3[i] < 0 || options3[i] > 2)
    {
      return false;
    }
  }

  int op_mode = std::stoi(options[0]);
  int amplitude_id = std::stoi(options[1]);
  int nodeset_id = std::stoi(options[3]);
  int bc_id = 1;
  int time_delay_id = 1;
  int displacement_id = 1;
  int name_id = 1;

  if (!bcs_data.empty())
  {
    bc_id = bcs_data.back()[0] + 1;
  }
  if (!time_delay_data.empty())
  {
    time_delay_id = std::stoi(time_delay_data.back()[0]) + 1;
  }
  if (!displacement_data.empty())
  {
    displacement_id = int(displacement_data.back()[0]) + 1;
  }
  if (!name_data.empty())
  {
    name_id = std::stoi(name_data.back()[0]) + 1;
  }

  add_time_delay(std::to_string(time_delay_id), options[2]);
  add_displacement(double(displacement_id), options2);
  add_mode(displacement_id, options3);
  add_name(std::to_string(name_id), options[4]);
  return add_bc(bc_id, op_mode, amplitude_id, time_delay_id, nodeset_id, displacement_id, name_id);
}

bool CoreBCsDisplacements::add_bc(int bc_id, int op_mode, int amplitude_id, int time_delay_id, int nodeset_id, int displacement_id, int name_id)
{
  std::vector<int> v = {bc_id, op_mode, amplitude_id, time_delay_id, nodeset_id, displacement_id, name_id};
  bcs_data.push_back(v);
  return true;
}

bool CoreBCsDisplacements::modify_bc(int bc_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3, std::vector<int> options_marker)
{
  // markers: op, amplitude, delay, nodeset, dofs 1..6, name
  if (options.size() != 5 || options2.size() != 6 || options3.size() != 6 || options_marker.size() != 11)
  {
    return false;
  }

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return false;
  }

  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
  int displacement_data_id = get_displacement_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  int mode_data_id = get_mode_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  int name_data_id = get_name_data_id_from_name_id(bcs_data[bcs_data_id][6]);
  if (time_delay_data_id == -1 || displacement_data_id == -1 || mode_data_id == -1 || name_data_id == -1)
  {
    return false;
  }

  for (size_t i = 0; i < 6; i++)
  {
    if (options_marker[i + 4] == 1)
    {
      if (options3[i] < 0 || options3[i] > 2)
      {
        return false;
      }
    }
  }

  // Parse all requested integer changes before modifying stored data.
  int op_mode = bcs_data[bcs_data_id][1];
  int amplitude_id = bcs_data[bcs_data_id][2];
  int nodeset_id = bcs_data[bcs_data_id][4];
  if (options_marker[0] == 1)
  {
    op_mode = std::stoi(options[0]);
  }
  if (options_marker[1] == 1)
  {
    amplitude_id = std::stoi(options[1]);
  }
  if (options_marker[3] == 1)
  {
    nodeset_id = std::stoi(options[3]);
  }

  bcs_data[bcs_data_id][1] = op_mode;
  bcs_data[bcs_data_id][2] = amplitude_id;
  bcs_data[bcs_data_id][4] = nodeset_id;
  if (options_marker[2] == 1)
  {
    time_delay_data[time_delay_data_id][1] = options[2];
  }
  for (size_t i = 0; i < 6; i++)
  {
    if (options_marker[i + 4] == 1)
    {
      displacement_data[displacement_data_id][i + 1] = options2[i];
      mode_data[mode_data_id][i + 1] = options3[i];
    }
  }
  if (options_marker[10] == 1)
  {
    name_data[name_data_id][1] = options[4];
  }
  return true;
}

bool CoreBCsDisplacements::delete_bc(int bc_id)
{
  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return false;
  }

  int sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
  if (sub_data_id != -1)
  {
    time_delay_data.erase(time_delay_data.begin() + sub_data_id);
  }
  sub_data_id = get_displacement_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  if (sub_data_id != -1)
  {
    displacement_data.erase(displacement_data.begin() + sub_data_id);
  }
  sub_data_id = get_mode_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  if (sub_data_id != -1)
  {
    mode_data.erase(mode_data.begin() + sub_data_id);
  }
  sub_data_id = get_name_data_id_from_name_id(bcs_data[bcs_data_id][6]);
  if (sub_data_id != -1)
  {
    name_data.erase(name_data.begin() + sub_data_id);
  }

  bcs_data.erase(bcs_data.begin() + bcs_data_id);
  return true;
}

bool CoreBCsDisplacements::add_displacement(double displacement_id, std::vector<double> values)
{
  if (values.size() != 6)
  {
    return false;
  }

  std::vector<double> v = {displacement_id, values[0], values[1], values[2], values[3], values[4], values[5]};
  displacement_data.push_back(v);
  return true;
}

bool CoreBCsDisplacements::add_mode(int displacement_id, std::vector<int> modes)
{
  if (modes.size() != 6)
  {
    return false;
  }

  std::vector<int> v = {displacement_id, modes[0], modes[1], modes[2], modes[3], modes[4], modes[5]};
  mode_data.push_back(v);
  return true;
}

bool CoreBCsDisplacements::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  name_data.push_back(v);
  return true;
}

bool CoreBCsDisplacements::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

int CoreBCsDisplacements::get_bcs_data_id_from_bc_id(int bc_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    if (bcs_data[i][0]==bc_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreBCsDisplacements::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    if (time_delay_data[i][0]==std::to_string(time_delay_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreBCsDisplacements::get_displacement_data_id_from_displacement_id(int displacement_id)
{
  int return_int = -1;
  for (size_t i = 0; i < displacement_data.size(); i++)
  {
    if (displacement_data[i][0] == double(displacement_id))
    {
      return_int = int(i);
    }
  }
  return return_int;
}

int CoreBCsDisplacements::get_name_data_id_from_name_id(int name_id)
{
  int return_int = -1;
  for (size_t i = 0; i < name_data.size(); i++)
  {
    if (name_data[i][0] == std::to_string(name_id))
    {
      return_int = int(i);
    }
  }
  return return_int;
}

int CoreBCsDisplacements::get_mode_data_id_from_displacement_id(int displacement_id)
{
  int return_int = -1;
  for (size_t i = 0; i < mode_data.size(); i++)
  {
    if (mode_data[i][0] == displacement_id)
    {
      return_int = int(i);
    }
  }
  return return_int;
}

std::string CoreBCsDisplacements::get_bc_parameter_export(int bc_id)
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;

  int bc_data_id;
  int sub_data_id;
  std::string str_temp = "";
  bc_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bc_data_id == -1)
  {
    return "";
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (sub_data_id == -1)
  {
    return "";
  }
  if (bcs_data[bc_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (bcs_data[bc_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (bcs_data[bc_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_amplitudes_iface.get_amplitude_name(bcs_data[bc_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (bcs_data[bc_data_id][2] != -1 &&
      time_delay_data[sub_data_id][1] != "")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  return str_temp;
}

std::string CoreBCsDisplacements::get_bc_parameter_fixed_export(int bc_id)
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;

  int bc_data_id;
  int sub_data_id;
  std::string str_temp = "";
  bc_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bc_data_id == -1)
  {
    return "";
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (sub_data_id == -1)
  {
    return "";
  }
  if (bcs_data[bc_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (bcs_data[bc_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (bcs_data[bc_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_amplitudes_iface.get_amplitude_name(bcs_data[bc_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (bcs_data[bc_data_id][2] != -1 &&
      time_delay_data[sub_data_id][1] != "")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  str_temp.append(",FIXED");
  return str_temp;
}

bool CoreBCsDisplacements::check_dof_fixed(int bc_id, int dof)
{
  if (dof < 1 || dof > 6)
  {
    return false;
  }

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return false;
  }

  int mode_data_id = get_mode_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  if (mode_data_id == -1)
  {
    return false;
  }

  return mode_data[mode_data_id][dof] == 2;
}

bool CoreBCsDisplacements::check_fixed(int bc_id)
{
  for (int dof = 1; dof <= 6; dof++)
  {
    if (check_dof_fixed(bc_id, dof))
    {
      return true;
    }
  }
  return false;
}

std::string CoreBCsDisplacements::get_bc_export(int bc_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return "";
  }

  int displacement_data_id = get_displacement_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  int mode_data_id = get_mode_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);

  if (displacement_data_id == -1 ||
      mode_data_id == -1 ||
      time_delay_data_id == -1)
  {
    return "";
  }

  int nodeset_id = bcs_data[bcs_data_id][4];
  if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  std::string prescribed;
  std::string fixed;

  for (int dof = 1; dof <= 6; dof++)
  {
    int mode = mode_data[mode_data_id][dof];
    if (mode < 0 || mode > 2)
    {
      return "";
    }

    if (mode == 1)
    {
      prescribed.append(
          nodeset_name + "," +
          std::to_string(dof) + "," +
          std::to_string(dof) + "," +
          ccx_application_iface.to_string_scientific(
              displacement_data[displacement_data_id][dof]) + "\n");
    }
    else if (mode == 2)
    {
      fixed.append(
          nodeset_name + "," +
          std::to_string(dof) + "," +
          std::to_string(dof) + "\n");
    }
  }

  std::string str_temp;

  if (!prescribed.empty())
  {
    str_temp.append("*BOUNDARY");
    str_temp.append(get_bc_parameter_export(bc_id));
    str_temp.append("\n");
    str_temp.append(prescribed);
  }

  if (!fixed.empty())
  {
    str_temp.append("*BOUNDARY");
    str_temp.append(get_bc_parameter_fixed_export(bc_id));
    str_temp.append("\n");
    str_temp.append(fixed);
  }

  // Preserve an explicit reset even when this BC has no active DOFs.
  if (prescribed.empty() && fixed.empty() && bcs_data[bcs_data_id][1] == 1)
  {
    str_temp.append("*BOUNDARY,OP=NEW\n");
  }

  return str_temp;
}

std::string CoreBCsDisplacements::get_hbc_export(int bc_id)
{
  CalculiXMeshInterface ccx_mesh_iface;

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return "";
  }

  int mode_data_id = get_mode_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);

  if (mode_data_id == -1 || time_delay_data_id == -1)
  {
    return "";
  }

  int nodeset_id = bcs_data[bcs_data_id][4];
  if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  std::string constrained;

  for (int dof = 1; dof <= 6; dof++)
  {
    int mode = mode_data[mode_data_id][dof];
    if (mode < 0 || mode > 2)
    {
      return "";
    }

    if (mode != 0)
    {
      constrained.append(
          nodeset_name + "," +
          std::to_string(dof) + "," +
          std::to_string(dof) + "\n");
    }
  }

  if (constrained.empty())
  {
    return "";
  }

  std::string str_temp = "*BOUNDARY";
  str_temp.append(get_bc_parameter_export(bc_id));
  str_temp.append("\n");
  str_temp.append(constrained);

  return str_temp;
}

std::string CoreBCsDisplacements::get_initial_displacement_export(int bc_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return "";
  }

  int displacement_data_id = get_displacement_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);
  int mode_data_id = get_mode_data_id_from_displacement_id(bcs_data[bcs_data_id][5]);

  if (displacement_data_id == -1 || mode_data_id == -1)
  {
    return "";
  }

  int nodeset_id = bcs_data[bcs_data_id][4];
  if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  std::string initial;

  for (int dof = 1; dof <= 6; dof++)
  {
    int mode = mode_data[mode_data_id][dof];
    if (mode < 0 || mode > 2)
    {
      return "";
    }

    if (mode != 0)
    {
      double value = 0.0;
      if (mode == 1)
      {
        value = displacement_data[displacement_data_id][dof];
      }

      initial.append(
          nodeset_name + "," +
          std::to_string(dof) + "," +
          ccx_application_iface.to_string_scientific(value) + "\n");
    }
  }

  if (initial.empty())
  {
    return "";
  }

  return "*INITIAL CONDITIONS,TYPE=DISPLACEMENT\n" + initial;
}

std::string CoreBCsDisplacements::print_data()
{
  std::string str_return = "\n CoreBCsDisplacements bcs_data: \n";
  str_return.append("bc_id, OP MODE, amplitude_id, time_delay_id, nodeset_id, displacement_id, name_id\n");
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    for (size_t ii = 0; ii < bcs_data[i].size(); ii++)
    {
      str_return.append(std::to_string(bcs_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreBCsDisplacements time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value\n");
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + "\n");
  }

  str_return.append("\n CoreBCsDisplacements displacement_data: \n");
  str_return.append("displacement_id, dof_1, dof_2, dof_3, dof_4, dof_5, dof_6\n");
  for (size_t i = 0; i < displacement_data.size(); i++)
  {
    for (size_t ii = 0; ii < displacement_data[i].size(); ii++)
    {
      str_return.append(std::to_string(displacement_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreBCsDisplacements mode_data: \n");
  str_return.append("displacement_id, mode_1, mode_2, mode_3, mode_4, mode_5, mode_6\n");
  for (size_t i = 0; i < mode_data.size(); i++)
  {
    for (size_t ii = 0; ii < mode_data[i].size(); ii++)
    {
      str_return.append(std::to_string(mode_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreBCsDisplacements name_data: \n");
  str_return.append("name_id, name\n");
  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + "\n");
  }
  return str_return;
}