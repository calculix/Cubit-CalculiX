#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "CubitInterface.hpp"
#include "MeshExportInterface.hpp"

CoreLoadsSurfaceTraction::CoreLoadsSurfaceTraction()
{}

CoreLoadsSurfaceTraction::~CoreLoadsSurfaceTraction()
{}

bool CoreLoadsSurfaceTraction::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsSurfaceTraction::update()
{ 
  return true;
}

bool CoreLoadsSurfaceTraction::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  force_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreLoadsSurfaceTraction::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsSurfaceTraction::create_load(std::vector<std::string> options, std::vector<double> options2)
{
  int load_id;
  int load_last;
  int sub_id;
  int sub_last;
  int op_mode;
  int amplitude_id;
  int sideset_id;
  int time_delay_id;
  int force_id;
  int name_id;
  
  if (loads_data.size()==0)
  {
    load_id = 1;
  }
  else
  {
    load_last = int(loads_data.size()) - 1;
    load_id = loads_data[load_last][0] + 1;
  }

  // OP MODE
  op_mode = std::stoi(options[0]);
  
  // AMPLITUDE
  amplitude_id = std::stoi(options[1]);

  // time_delay
  if (time_delay_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(time_delay_data.size()) - 1;
    sub_id = std::stoi(time_delay_data[sub_last][0]) + 1;
  }
  time_delay_id = sub_id;
  this->add_time_delay(std::to_string(sub_id), options[2]);

  // SIDESET
  sideset_id = std::stoi(options[3]);
  
  // FORCES
  if (force_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(force_data.size()) - 1;
    sub_id = int(force_data[sub_last][0]) + 1;
  }
  force_id = sub_id;
  this->add_force(double(sub_id), options2[0], options2[1], options2[2]);

  // name
  if (name_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(name_data.size()) - 1;
    sub_id = std::stoi(name_data[sub_last][0]) + 1;
  }
  name_id = sub_id;
  this->add_name(std::to_string(sub_id), options[4]);

  this->add_load(load_id, op_mode, amplitude_id, time_delay_id, sideset_id, force_id, name_id);
  return true;
}

bool CoreLoadsSurfaceTraction::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int sideset_id, int force_id, int name_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, sideset_id, force_id, name_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsSurfaceTraction::modify_load(int load_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  
  if (loads_data_id == -1)
  {
    return false;
  } else {
    // OP MODE
    if (options_marker[0]==1)
    {
      loads_data[loads_data_id][1] = std::stoi(options[0]);
    }
    // AMPLITUDE
    if (options_marker[1]==1)
    {
      loads_data[loads_data_id][2] = std::stoi(options[1]);
    }
    // time delay
    if (options_marker[2]==1)
    {
      sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
      time_delay_data[sub_data_id][1] = options[2];
    }
    // SIDESET
    if (options_marker[3]==1)
    {
      loads_data[loads_data_id][4] = std::stoi(options[3]);
    }
    // FORCES
    if (options_marker[4]==1)
    {
      sub_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
      force_data[sub_data_id][1] = options2[0];
    }
    if (options_marker[5]==1)
    {
      sub_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
      force_data[sub_data_id][2] = options2[1];
    }
    if (options_marker[6]==1)
    {
      sub_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
      force_data[sub_data_id][3] = options2[2];
    }
    // name
    if (options_marker[7]==1)
    {
      sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][6]);
      name_data[sub_data_id][1] = options[4];
    }
    return true;
  }
}

bool CoreLoadsSurfaceTraction::delete_load(int load_id)
{
  int sub_data_id;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  } else {
    // time delay
    sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
    if (sub_data_id != -1){
      time_delay_data.erase(time_delay_data.begin() + sub_data_id);
    }
    // forces
    sub_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      force_data.erase(force_data.begin() + sub_data_id);
    }
    // name
    sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][6]);
    if (sub_data_id != -1){
      name_data.erase(name_data.begin() + sub_data_id);
    }

    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsSurfaceTraction::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

bool CoreLoadsSurfaceTraction::add_force(double force_id, double dof_1_value, double dof_2_value, double dof_3_value)
{
  std::vector<double> v = {force_id, dof_1_value, dof_2_value, dof_3_value};
      
  force_data.push_back(v);

  return true;
}

bool CoreLoadsSurfaceTraction::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

int CoreLoadsSurfaceTraction::get_loads_data_id_from_load_id(int load_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < loads_data.size(); i++)
  {
    if (loads_data[i][0]==load_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsSurfaceTraction::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
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

int CoreLoadsSurfaceTraction::get_force_data_id_from_force_id(int temperature_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < force_data.size(); i++)
  {
    if (force_data[i][0]==double(temperature_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsSurfaceTraction::get_name_data_id_from_name_id(int name_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < name_data.size(); i++)
  {
    if (name_data[i][0]==std::to_string(name_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::vector<double> CoreLoadsSurfaceTraction::calc_consistent_load(std::vector<int> connectivity, double load)
{
  std::vector<double> consistent_load;

  if (connectivity.size()==3)  //linear tri
  {
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
  }else if (connectivity.size()==6) //quadratic tri
  {
    consistent_load.push_back(0.);
    consistent_load.push_back(0.);
    consistent_load.push_back(0.);
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
  }else if (connectivity.size()==4)  //linear quad
  {
    consistent_load.push_back(1./4. * load);
    consistent_load.push_back(1./4. * load);
    consistent_load.push_back(1./4. * load);
    consistent_load.push_back(1./4. * load);
  }else if (connectivity.size()==8) //quadratic quad
  {
    consistent_load.push_back(-1./12. * load);
    consistent_load.push_back(-1./12. * load);
    consistent_load.push_back(-1./12. * load);
    consistent_load.push_back(-1./12. * load);
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
    consistent_load.push_back(1./3. * load);
  }
  
  return consistent_load;
}
  

std::string CoreLoadsSurfaceTraction::get_load_export(int load_id)
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;
  CalculiXApplicationInterface ccx_application_iface;

  int load_data_id;
  int sub_data_id;
  int force_data_id;
  std::vector<int> node_ids;
  std::vector<int> tri_ids;
  std::vector<int> quad_ids;
  std::vector<std::vector<double>> force_values; //will be mapped with the node ids
  std::vector<double> tri_area; //will be mapped with the tri ids
  std::vector<double> quad_area; //will be mapped with the quad ids
  double area = 0; // sum area of sideset

  std::string str_temp = "";
  
  load_data_id = get_loads_data_id_from_load_id(load_id);
  str_temp.append("*CLOAD");
  if (loads_data[load_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (loads_data[load_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (loads_data[load_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_amplitudes_iface.get_amplitude_name(loads_data[load_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[load_data_id][3]);
  if (time_delay_data[sub_data_id][1]!="")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  str_temp.append("\n");

  force_data_id = get_force_data_id_from_force_id(loads_data[load_data_id][5]);
  if (force_data_id == -1)
  {
    return "ERROR: SOMETHING WENT WRONG WITH SURFACE TRACTION ID " + std::to_string(load_id);
  }
  
  
  //get all faces in the sideset
  node_ids = CubitInterface::parse_cubit_list("node", "all in sideset " + std::to_string(loads_data[load_data_id][4])); 
  tri_ids = CubitInterface::parse_cubit_list("tri", "all in sideset " + std::to_string(loads_data[load_data_id][4])); 
  quad_ids = CubitInterface::parse_cubit_list("face", "all in sideset " + std::to_string(loads_data[load_data_id][4])); 

  /*
  str_temp.append("node_ids\n");
  for (size_t i = 0; i < node_ids.size(); i++)
  {
    str_temp.append(std::to_string(node_ids[i]) + " ");
  }
  str_temp.append("\n");

  str_temp.append("tri_ids\n");
  for (size_t i = 0; i < tri_ids.size(); i++)
  {
    str_temp.append(std::to_string(tri_ids[i]) + " ");
  }
  str_temp.append("\n");

  str_temp.append("quad_ids\n");
  for (size_t i = 0; i < quad_ids.size(); i++)
  {
    str_temp.append(std::to_string(quad_ids[i]) + " ");
  }
  str_temp.append("\n");
  */

  //sum area
  tri_area = CubitInterface::get_quality_values("tri", tri_ids, "element area");
  quad_area = CubitInterface::get_quality_values("quad", quad_ids, "element area");
 
  //str_temp.append("tri_area\n");
  for (size_t i = 0; i < tri_area.size(); i++)
  {
    //str_temp.append(std::to_string(tri_area[i]) + " ");
    area = area + tri_area[i];
  }
  //str_temp.append("\n");

  //str_temp.append("quad_area\n");
  for (size_t i = 0; i < quad_area.size(); i++)
  {
    //str_temp.append(std::to_string(quad_area[i]) + " ");
    area = area + quad_area[i];
  }
  //str_temp.append("\n");

  //str_temp.append("area\n");
  //str_temp.append(std::to_string(area) + "\n");    

  //assign forces to node ids
  for (size_t i = 0; i < node_ids.size(); i++)
  {
    force_values.push_back({0.,0.,0.});
  }

  auto p = sort_permutation(node_ids);
  this->apply_permutation(node_ids, p);

  for (size_t i = 0; i < tri_ids.size(); i++)
  {
    std::vector<int> connectivity = CubitInterface::get_expanded_connectivity("tri",tri_ids[i]);
    for (size_t ii = 1; ii < 4; ii++)
    {
      std::vector<double> tmp_load = calc_consistent_load(connectivity,(force_data[force_data_id][ii] * tri_area[i]/area));
      if((tmp_load.size()!=0)&&(tmp_load.size()==connectivity.size()))
      {
        for (size_t iii = 0; iii < connectivity.size(); iii++)
        {
          if (std::binary_search(node_ids.begin(), node_ids.end(), connectivity[iii]))
          {
            auto lower = std::lower_bound(node_ids.begin(), node_ids.end(), connectivity[iii]);
            force_values[lower - node_ids.begin()][ii-1] += tmp_load[iii];
          }
        }
      }
    }
  }

  for (size_t i = 0; i < quad_ids.size(); i++)
  {
    std::vector<int> connectivity = CubitInterface::get_expanded_connectivity("quad",quad_ids[i]);
    for (size_t ii = 1; ii < 4; ii++)
    {
      std::vector<double> tmp_load = calc_consistent_load(connectivity,(force_data[force_data_id][ii] * quad_area[i]/area));
      if((tmp_load.size()!=0)&&(tmp_load.size()==connectivity.size()))
      {
        for (size_t iii = 0; iii < connectivity.size(); iii++)
        {
          if (std::binary_search(node_ids.begin(), node_ids.end(), connectivity[iii]))
          {
            auto lower = std::lower_bound(node_ids.begin(), node_ids.end(), connectivity[iii]);
            force_values[lower - node_ids.begin()][ii-1] += tmp_load[iii];
          }
        }
      }
    }
  }

  // write node ids and forces
  for (size_t i = 0; i < force_values.size(); i++)
  {
    /*
    str_temp.append("force_values[" + std::to_string(i)+ "][0] " + std::to_string(force_values[i][0])+ "\n");
    str_temp.append("force_values[" + std::to_string(i)+ "][1] " + std::to_string(force_values[i][1])+ "\n");
    str_temp.append("force_values[" + std::to_string(i)+ "][2] " + std::to_string(force_values[i][2])+ "\n");
    */
    if (force_values[i][0] != 0.)
    {
      str_temp.append(std::to_string(node_ids[i]) + ",1," + ccx_application_iface.to_string_scientific(force_values[i][0])+ "\n");
    }
    if (force_values[i][1] != 0.)
    {
      str_temp.append(std::to_string(node_ids[i]) + ",2," + ccx_application_iface.to_string_scientific(force_values[i][1])+ "\n");
    }
    if (force_values[i][2] != 0.)
    {
      str_temp.append(std::to_string(node_ids[i]) + ",3," + ccx_application_iface.to_string_scientific(force_values[i][2])+ "\n");
    }
  }

  return str_temp;
}

std::string CoreLoadsSurfaceTraction::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsSurfaceTraction loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, sideset_id, force_id, name_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6])  + " \n");
  }

  str_return.append("\n CoreLoadsSurfaceTraction time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsSurfaceTraction force_data: \n");
  str_return.append("force_id,dof_1_value,dof_2_value,dof_3_value  \n");

  for (size_t i = 0; i < force_data.size(); i++)
  {
    str_return.append(std::to_string(force_data[i][0]) + " " + std::to_string(force_data[i][1]) + " " + std::to_string(force_data[i][2]) + " " + std::to_string(force_data[i][3]) + " \n");
  }
  
  str_return.append("\n CoreLoadsSurfaceTraction name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }

  return str_return;
}



//sorting of vectors
template <typename T> 
std::vector<std::size_t> CoreLoadsSurfaceTraction::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreLoadsSurfaceTraction::apply_permutation(
    std::vector<T>& vec,
    const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        if (done[i])
        {
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}