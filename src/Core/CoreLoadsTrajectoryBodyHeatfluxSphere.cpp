#include "CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <algorithm>
#include "StopWatch.hpp"

CoreLoadsTrajectoryBodyHeatfluxSphere::CoreLoadsTrajectoryBodyHeatfluxSphere()
{}

CoreLoadsTrajectoryBodyHeatfluxSphere::~CoreLoadsTrajectoryBodyHeatfluxSphere()
{}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::update()
{ 
  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::reset()
{
  loads_data.clear();
  fire_ray_surface_data.clear();
  time_data.clear();
  direction_data.clear();
  magnitude_data.clear();
  radius_data.clear();
  depth_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::create_load(std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  int load_id;
  int load_last;
  int sub_id;
  int sub_last;
  int op_mode;
  int curve_id;
  int vertex_id;
  int fire_ray_surface_id;
  int direction_id;
  int magnitude_id;
  int time_id;
  int radius_id;
  int depth_id;
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
  
  // curve
  curve_id = std::stoi(options[1]);

  // vertex
  vertex_id = std::stoi(options[2]);

  // fire ray
  fire_ray_surface_id = load_id;
  for (size_t i = 0; i < options2.size(); i++)
  {
    this->add_fire_ray_surface(fire_ray_surface_id, options2[i]);
  }
  
  // direction
  if (direction_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(direction_data.size()) - 1;
    sub_id = std::stoi(direction_data[sub_last][0]) + 1;
  }
  direction_id = sub_id;
  this->add_direction(std::to_string(sub_id), options[3], options[4], options[5]);

  // magnitude
  if (magnitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < magnitude_data.size(); i++)
    {
      sub_last = int(magnitude_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  magnitude_id = sub_id;
  for (size_t i = 0; i < options3[0].size(); i++)
  {
    this->add_magnitude(double(magnitude_id), options3[0][i]);
  }

  // time
  if (time_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(time_data.size()) - 1;
    sub_id = std::stoi(time_data[sub_last][0]) + 1;
  }
  time_id = sub_id;
  this->add_time(std::to_string(sub_id), options[6], options[7]);
  
  // radius
  if (radius_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < radius_data.size(); i++)
    {
      sub_last = int(radius_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  radius_id = sub_id;
  for (size_t i = 0; i < options3[1].size(); i++)
  {
    this->add_radius(double(radius_id), options3[1][i]);
  }

  // depth
  if (depth_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < depth_data.size(); i++)
    {
      sub_last = int(depth_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  depth_id = sub_id;
  for (size_t i = 0; i < options3[1].size(); i++)
  {
    this->add_depth(double(depth_id), options3[2][i]);
  }

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
  this->add_name(std::to_string(sub_id), options[8]);

  this->add_load(load_id, op_mode, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id, radius_id, depth_id, name_id);
  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_load(int load_id, int op_mode, int curve_id, int vertex_id, int fire_ray_surface_id, int direction_id, int magnitude_id, int time_id, int radius_id, int depth_id, int name_id)
{
  std::vector<int> v = {load_id, op_mode, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id, radius_id, depth_id, name_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3)
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
    // curve
    if (options_marker[1]==1)
    {
      loads_data[loads_data_id][2] = std::stoi(options[1]);
    }
    // vertex
    if (options_marker[2]==1)
    {
      loads_data[loads_data_id][3] = std::stoi(options[2]);
    }
    // fire ray
    if (options_marker[3]==1)
    {
      // surface ids
      sub_data_ids = get_fire_ray_surface_data_ids_from_fire_ray_surface_id(loads_data[loads_data_id][4]);
      if (options2.size()!=0)
      {
        if (options2.size()==sub_data_ids.size())
        {
          for (size_t i = 0; i < options2.size(); i++)
          {
            fire_ray_surface_data[sub_data_ids[i]][0] = loads_data[loads_data_id][4];
            fire_ray_surface_data[sub_data_ids[i]][1] = options2[i];
          }
        }else{
          // first delete and then make a push back
          // delete from back to begin so that we don't have to care about mismatching id's
          for (size_t i = sub_data_ids.size(); i > 0; i--)
          {
            fire_ray_surface_data.erase(fire_ray_surface_data.begin() + sub_data_ids[i-1]);
          }
          
          for (size_t i = 0; i < options2.size(); i++)
          {
            add_fire_ray_surface(loads_data[loads_data_id][4],options2[i]);
          }
        }
      }
    }
    // direction
    if (options_marker[4]==1)
    {
      sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
      direction_data[sub_data_id][1] = options[4];
    }
    if (options_marker[5]==1)
    {
      sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
      direction_data[sub_data_id][2] = options[5];
    }
    if (options_marker[6]==1)
    {
      sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
      direction_data[sub_data_id][3] = options[6];
    }
    // magnitude
    if (options_marker[7]==1)
    {
      //sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
      //magnitude_data[sub_data_id][1] = options[7];
      sub_data_ids = get_magnitude_data_ids_from_magnitude_id(loads_data[loads_data_id][6]);
      if (options3[0].size()!=0)
      {
        if (options3[0].size()==sub_data_ids.size())
        {
          for (size_t i = 0; i < options3[0].size(); i++)
          {
            magnitude_data[sub_data_ids[i]][0] = loads_data[loads_data_id][6];
            magnitude_data[sub_data_ids[i]][1] = options3[0][i];
          }
        }else{
          // first delete and then make a push back
          // delete from back to begin so that we don't have to care about mismatching id's
          for (size_t i = sub_data_ids.size(); i > 0; i--)
          {
            magnitude_data.erase(magnitude_data.begin() + sub_data_ids[i-1]);
          }
          
          for (size_t i = 0; i < options3[0].size(); i++)
          {
            add_magnitude(double(loads_data[loads_data_id][6]),options3[0][i]);
          }
        }
      }
    }
    // time
    if (options_marker[8]==1)
    {
      sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
      time_data[sub_data_id][1] = options[7];
    }
    if (options_marker[9]==1)
    {
      sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
      time_data[sub_data_id][2] = options[8];
    }
    // radius
    if (options_marker[10]==1)
    {
      //sub_data_id = get_radius_data_id_from_radius_id(loads_data[loads_data_id][8]);
      //radius_data[sub_data_id][1] = options[10];
      sub_data_ids = get_radius_data_ids_from_radius_id(loads_data[loads_data_id][8]);
      if (options3[1].size()!=0)
      {
        if (options3[1].size()==sub_data_ids.size())
        {
          for (size_t i = 0; i < options3[1].size(); i++)
          {
            radius_data[sub_data_ids[i]][0] = loads_data[loads_data_id][8];
            radius_data[sub_data_ids[i]][1] = options3[1][i];
          }
        }else{
          // first delete and then make a push back
          // delete from back to begin so that we don't have to care about mismatching id's
          for (size_t i = sub_data_ids.size(); i > 0; i--)
          {
            radius_data.erase(radius_data.begin() + sub_data_ids[i-1]);
          }
          
          for (size_t i = 0; i < options3[1].size(); i++)
          {
            add_radius(double(loads_data[loads_data_id][8]),options3[1][i]);
          }
        }
      }
    }
    // depth
    if (options_marker[11]==1)
    {
      sub_data_ids = get_depth_data_ids_from_depth_id(loads_data[loads_data_id][9]);
      if (options3[2].size()!=0)
      {
        if (options3[2].size()==sub_data_ids.size())
        {
          for (size_t i = 0; i < options3[2].size(); i++)
          {
            depth_data[sub_data_ids[i]][0] = loads_data[loads_data_id][9];
            depth_data[sub_data_ids[i]][1] = options3[2][i];
          }
        }else{
          // first delete and then make a push back
          // delete from back to begin so that we don't have to care about mismatching id's
          for (size_t i = sub_data_ids.size(); i > 0; i--)
          {
            depth_data.erase(depth_data.begin() + sub_data_ids[i-1]);
          }
          
          for (size_t i = 0; i < options3[2].size(); i++)
          {
            add_depth(double(loads_data[loads_data_id][9]),options3[2][i]);
          }
        }
      }
    }
    // name
    if (options_marker[12]==1)
    {
      sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][10]);
      name_data[sub_data_id][1] = options[9];
    }
    return true;
  }
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::delete_load(int load_id)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  } else {
    // time
    sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
    if (sub_data_id != -1){
      time_data.erase(time_data.begin() + sub_data_id);
    }
    // radius
    sub_data_ids = get_radius_data_ids_from_radius_id(loads_data[loads_data_id][8]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      radius_data.erase(radius_data.begin() + sub_data_ids[i-1]);
    }
    // depth
    sub_data_ids = get_depth_data_ids_from_depth_id(loads_data[loads_data_id][9]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      depth_data.erase(depth_data.begin() + sub_data_ids[i-1]);
    }
    // direction
    sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      direction_data.erase(direction_data.begin() + sub_data_id);
    }
    // magnitude
    sub_data_ids = get_magnitude_data_ids_from_magnitude_id(loads_data[loads_data_id][6]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      magnitude_data.erase(magnitude_data.begin() + sub_data_ids[i-1]);
    }
    // fire ray surfaces
    sub_data_ids = get_fire_ray_surface_data_ids_from_fire_ray_surface_id(loads_data[loads_data_id][4]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      fire_ray_surface_data.erase(fire_ray_surface_data.begin() + sub_data_ids[i-1]);
    }
    // name
    sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][10]);
    if (sub_data_id != -1){
      name_data.erase(name_data.begin() + sub_data_id);
    }
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_time(std::string time_id, std::string t_begin, std::string t_end)
{
  std::vector<std::string> v = {time_id, t_begin, t_end};
      
  time_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_radius(double radius_id, double radius)
{
  std::vector<double> v = {radius_id, radius};
      
  radius_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_depth(double depth_id, double depth)
{
  std::vector<double> v = {depth_id, depth};
      
  depth_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_direction(std::string direction_id, std::string x, std::string y, std::string z)
{
  std::vector<std::string> v = {direction_id, x, y, z};
      
  direction_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_magnitude(double magnitude_id, double magnitude_value)
{
  std::vector<double> v = {magnitude_id, magnitude_value};
      
  magnitude_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_fire_ray_surface(int fire_ray_surface_id, int surface_id)
{
  std::vector<int> v = {fire_ray_surface_id, surface_id};
      
  fire_ray_surface_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

int CoreLoadsTrajectoryBodyHeatfluxSphere::get_loads_data_id_from_load_id(int load_id)
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

int CoreLoadsTrajectoryBodyHeatfluxSphere::get_time_data_id_from_time_id(int time_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < time_data.size(); i++)
  {
    if (time_data[i][0]==std::to_string(time_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_radius_data_ids_from_radius_id(int radius_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < radius_data.size(); i++)
  {
    if (radius_data[i][0]==double(radius_id))
    {
      return_int.push_back(int(i));
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_depth_data_ids_from_depth_id(int depth_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < depth_data.size(); i++)
  {
    if (depth_data[i][0]==double(depth_id))
    {
      return_int.push_back(int(i));
    }  
  }
  return return_int;
}

int CoreLoadsTrajectoryBodyHeatfluxSphere::get_direction_data_id_from_direction_id(int direction_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < direction_data.size(); i++)
  {
    if (direction_data[i][0]==std::to_string(direction_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_magnitude_data_ids_from_magnitude_id(int magnitude_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    if (magnitude_data[i][0]==double(magnitude_id))
    {
        return_int.push_back(int(i));
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_fire_ray_surface_data_ids_from_fire_ray_surface_id(int fire_surface_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < fire_ray_surface_data.size(); i++)
  {
    if (fire_ray_surface_data[i][0]==fire_surface_id)
    {
        return_int.push_back(int(i));
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_fire_ray_surface_ids_from_fire_ray_surface_id(int fire_surface_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < fire_ray_surface_data.size(); i++)
  {
    if (fire_ray_surface_data[i][0]==fire_surface_id)
    {
        return_int.push_back(int(fire_ray_surface_data[i][1]));
    }  
  }
  return return_int;
}

int CoreLoadsTrajectoryBodyHeatfluxSphere::get_name_data_id_from_name_id(int name_id)
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

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_node_ids(int load_id)
{
  std::vector<int> node_ids;
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    // check if curve is meshed
    if (CubitInterface::is_meshed("Curve",loads_data[load_data_id][2]))
    {
      std::vector<int> edge_ids;
      edge_ids = CubitInterface::get_curve_edges(loads_data[load_data_id][2]);
      for (size_t i = 0; i < edge_ids.size(); i++)
      {
        std::vector<int> tmp_node_ids = CubitInterface::parse_cubit_list("node","in edge " + std::to_string(edge_ids[i]));
        if (i==0)
        {
          node_ids.push_back(tmp_node_ids[0]);
        }
        node_ids.push_back(tmp_node_ids[1]);
      }
      //change order depending on the vertex id
      int vertex_node_id = CubitInterface::get_vertex_node(loads_data[load_data_id][3]);
      
      if (vertex_node_id == node_ids[node_ids.size()-1])
      {
        std::reverse(node_ids.begin(), node_ids.end());
      }
    }
  }

  return node_ids;
}

std::vector<int> CoreLoadsTrajectoryBodyHeatfluxSphere::get_edge_ids(int load_id)
{
  std::vector<int> edge_ids;
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    edge_ids = CubitInterface::get_curve_edges(loads_data[load_data_id][2]);
  }

  return edge_ids;
}

std::vector<std::vector<double>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_hit_coordinates(int load_id)
{
  std::vector<std::vector<double>> hit_coordinates;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    std::vector<int> surface_ids = this->get_fire_ray_surface_ids_from_fire_ray_surface_id(loads_data[load_data_id][4]);
    
    int direction_data_id = this->get_direction_data_id_from_direction_id(loads_data[load_data_id][5]);
    std::vector<int> radius_data_ids = this->get_radius_data_ids_from_radius_id(loads_data[load_data_id][8]);
    
    //get biggest radius for shooting the ray
    double radius = 0;
    for (size_t i = 0; i < radius_data_ids.size(); i++)
    {
      if (radius < radius_data[radius_data_ids[i]][1])
      {
        radius = radius_data[radius_data_ids[i]][1];
      }
    }

    std::array<double, 3> direction;

    direction[0] = ccx_iface->string_scientific_to_double(direction_data[direction_data_id][1]);
    direction[1] = ccx_iface->string_scientific_to_double(direction_data[direction_data_id][2]);
    direction[2] = ccx_iface->string_scientific_to_double(direction_data[direction_data_id][3]); 

    for (size_t i = 0; i < node_ids.size(); i++)
    {
      std::vector<double> hit_coord;
      std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
      std::pair<std::vector<std::array<double, 3>>, std::vector<int>> hit = CubitInterface::fire_ray(coord, direction, "surface", surface_ids, 1, radius);

      if (hit.first.size()>0)
      {
        hit_coord.push_back(hit.first[0][0]);
        hit_coord.push_back(hit.first[0][1]);
        hit_coord.push_back(hit.first[0][2]);
      }
      hit_coordinates.push_back(hit_coord);
    }
  }

  return hit_coordinates;
}

std::vector<std::vector<std::vector<int>>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_element_ids(int load_id)
{
  std::vector<std::vector<std::vector<int>>> selected_element_ids;
  //selected_element_ids[0] order by node
  //selected_element_ids[0][0] order by radius and depth
  //selected_element_ids[0][0][0] face ids

  std::vector<std::vector<std::vector<int>>> selected_tet_ids;
  //selected_tet_ids[0] order by node
  //selected_tet_ids[0][0] order by radius and depth
  //selected_tet_ids[0][0][0] tet ids

  std::vector<std::vector<std::vector<int>>> selected_hex_ids;
  //selected_hex_ids[0] order by node
  //selected_hex_ids[0][0] order by radius and depth
  //selected_hex_ids[0][0][0] tet ids

  std::vector<std::vector<std::vector<int>>> selected_wedge_ids;
  //selected_wedge_ids[0] order by node
  //selected_wedge_ids[0][0] order by radius and depth
  //selected_wedge_ids[0][0][0] tet ids

  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    
    std::vector<std::vector<double>> hit_coordinates = get_hit_coordinates(load_id);
    std::vector<int> surface_ids = this->get_fire_ray_surface_ids_from_fire_ray_surface_id(loads_data[load_data_id][4]);
    std::vector<int> tet_ids;
    std::vector<int> hex_ids;
    std::vector<int> wedge_ids;
    for (size_t i = 0; i < surface_ids.size(); i++)
    {
      int volume_id = volume_id = CubitInterface::get_owning_volume("surface" , surface_ids[i]);
      std::vector<int> tmp_tet_ids = CubitInterface::parse_cubit_list("tet","in volume " + std::to_string(volume_id));
      for (size_t ii = 0; ii < tmp_tet_ids.size(); ii++)
      {
        tet_ids.push_back(tmp_tet_ids[ii]);
      }
      std::vector<int> tmp_hex_ids = CubitInterface::parse_cubit_list("hex","in volume " + std::to_string(volume_id));
      for (size_t ii = 0; ii < tmp_hex_ids.size(); ii++)
      {
        hex_ids.push_back(tmp_hex_ids[ii]);
      }
      std::vector<int> tmp_wedge_ids = CubitInterface::parse_cubit_list("wedge","in volume " + std::to_string(volume_id));
      for (size_t ii = 0; ii < tmp_wedge_ids.size(); ii++)
      {
        wedge_ids.push_back(tmp_wedge_ids[ii]);
      }
    }
    
    std::vector<int> radius_data_ids = this->get_radius_data_ids_from_radius_id(loads_data[load_data_id][8]);
    std::vector<int> depth_data_ids = this->get_depth_data_ids_from_depth_id(loads_data[load_data_id][9]);
    
    // get all elements for a radius
    for (size_t i = 0; i < hit_coordinates.size(); i++) //loop over nodes
    {
      std::vector<std::vector<int>> tmp_tet_ids;
      std::vector<std::vector<int>> tmp_hex_ids;
      std::vector<std::vector<int>> tmp_wedge_ids;
      for (size_t ii = 0; ii < radius_data_ids.size(); ii++) //loop over radius
      {
        std::vector<int> tmp_selected_tet_ids;
        std::vector<int> tmp_selected_hex_ids;
        std::vector<int> tmp_selected_wedge_ids;

        double radius = 0;
        radius = radius_data[radius_data_ids[ii]][1];
        double depth = 0;
        depth = depth_data[depth_data_ids[ii]][1];
        
        if (hit_coordinates[i].size()>0)
        {
          for (size_t iii = 0; iii < tet_ids.size(); iii++)
          {
            std::array<double,3> center_point;
            center_point = CubitInterface::get_center_point("tet", tet_ids[iii]);

            double distance = sqrt(pow(center_point[0]-hit_coordinates[i][0],2)+pow(center_point[1]-hit_coordinates[i][1],2)+pow(center_point[2]-hit_coordinates[i][2],2));

            if (distance <= radius)
            {
              //check depth for all surfaces
              for (size_t iv = 0; iv < surface_ids.size(); iv++)
              {
                std::vector<double> measure = CubitInterface::measure_between_entities("tet", tet_ids[iii],"surface",surface_ids[iv]);
                if (measure[0] <= depth)
                {
                  tmp_selected_tet_ids.push_back(tet_ids[iii]);
                  break;
                }
              }
            }
          }
          for (size_t iii = 0; iii < hex_ids.size(); iii++)
          {
            std::array<double,3> center_point;
            center_point = CubitInterface::get_center_point("hex", hex_ids[iii]);

            double distance = sqrt(pow(center_point[0]-hit_coordinates[i][0],2)+pow(center_point[1]-hit_coordinates[i][1],2)+pow(center_point[2]-hit_coordinates[i][2],2));

            if (distance <= radius)
            {
              //check depth for all surfaces
              for (size_t iv = 0; iv < surface_ids.size(); iv++)
              {
                std::vector<double> measure = CubitInterface::measure_between_entities("hex", hex_ids[iii],"surface",surface_ids[iv]);
                if (measure[0] <= depth)
                {
                  tmp_selected_hex_ids.push_back(hex_ids[iii]);
                  break;
                }
              }
            }
          }
          for (size_t iii = 0; iii < wedge_ids.size(); iii++)
          {
            std::array<double,3> center_point;
            center_point = CubitInterface::get_center_point("wedge", hex_ids[iii]);

            double distance = sqrt(pow(center_point[0]-hit_coordinates[i][0],2)+pow(center_point[1]-hit_coordinates[i][1],2)+pow(center_point[2]-hit_coordinates[i][2],2));

            if (distance <= radius)
            {
              //check depth for all surfaces
              for (size_t iv = 0; iv < surface_ids.size(); iv++)
              {
                std::vector<double> measure = CubitInterface::measure_between_entities("wedge", wedge_ids[iii],"surface",surface_ids[iv]);
                if (measure[0] <= depth)
                {
                  tmp_selected_wedge_ids.push_back(wedge_ids[iii]);
                  break;
                }
              }
            }
          }
        }
        tmp_tet_ids.push_back(tmp_selected_tet_ids);
        tmp_hex_ids.push_back(tmp_selected_hex_ids);
        tmp_wedge_ids.push_back(tmp_selected_wedge_ids);
      }
      selected_tet_ids.push_back(tmp_tet_ids);
      selected_hex_ids.push_back(tmp_hex_ids);
      selected_wedge_ids.push_back(tmp_wedge_ids);
    } 

    // sort the elements out. this means if elements are in r1 and r2 and r1 is smaller
    // then all elements belonging to r1 will be deleted from r2, this will be repeated for all radius in ascending order
    for (size_t i = 0; i < selected_tet_ids.size(); i++)
    {
      for (size_t ii = 0; ii < selected_tet_ids[i].size()-1; ii++)
      {
        sort(begin(selected_tet_ids[i][ii]), end(selected_tet_ids[i][ii]));
        for (size_t iii = ii+1; iii < selected_tet_ids[i].size(); iii++)
        { 
          selected_tet_ids[i][iii].erase( remove_if( begin(selected_tet_ids[i][iii]),end(selected_tet_ids[i][iii]),
              [&](auto x){return binary_search(begin(selected_tet_ids[i][ii]),end(selected_tet_ids[i][ii]),x);}), end(selected_tet_ids[i][iii]) );
        }
      }
    }
    for (size_t i = 0; i < selected_hex_ids.size(); i++)
    {
      for (size_t ii = 0; ii < selected_hex_ids[i].size()-1; ii++)
      {
        sort(begin(selected_hex_ids[i][ii]), end(selected_hex_ids[i][ii]));
        for (size_t iii = ii+1; iii < selected_hex_ids[i].size(); iii++)
        { 
          selected_hex_ids[i][iii].erase( remove_if( begin(selected_hex_ids[i][iii]),end(selected_hex_ids[i][iii]),
              [&](auto x){return binary_search(begin(selected_hex_ids[i][ii]),end(selected_hex_ids[i][ii]),x);}), end(selected_hex_ids[i][iii]) );
        }
      }
    }
    for (size_t i = 0; i < selected_wedge_ids.size(); i++)
    {
      for (size_t ii = 0; ii < selected_wedge_ids[i].size()-1; ii++)
      {
        sort(begin(selected_wedge_ids[i][ii]), end(selected_wedge_ids[i][ii]));
        for (size_t iii = ii+1; iii < selected_wedge_ids[i].size(); iii++)
        { 
          selected_wedge_ids[i][iii].erase( remove_if( begin(selected_wedge_ids[i][iii]),end(selected_wedge_ids[i][iii]),
              [&](auto x){return binary_search(begin(selected_wedge_ids[i][ii]),end(selected_wedge_ids[i][ii]),x);}), end(selected_wedge_ids[i][iii]) );
        }
      }
    }
    // get global element ids
    for (size_t i = 0; i < hit_coordinates.size(); i++) //loop over nodes
    {
      std::vector<std::vector<int>> tmp_element_ids;
      for (size_t ii = 0; ii < radius_data_ids.size(); ii++) //loop over radius
      {
        std::vector<int> tmp_element_ids_2;
        for (size_t iii = 0; iii < selected_tet_ids[i][ii].size(); iii++)
        {
          int global_id = CubitInterface::get_global_element_id("tet",selected_tet_ids[i][ii][iii]);
          tmp_element_ids_2.push_back(global_id);
        }
        for (size_t iii = 0; iii < selected_hex_ids[i][ii].size(); iii++)
        {
          int global_id = CubitInterface::get_global_element_id("hex",selected_hex_ids[i][ii][iii]);
          tmp_element_ids_2.push_back(global_id);
        }
        for (size_t iii = 0; iii < selected_wedge_ids[i][ii].size(); iii++)
        {
          int global_id = CubitInterface::get_global_element_id("wedge",selected_wedge_ids[i][ii][iii]);
          tmp_element_ids_2.push_back(global_id);
        }
        tmp_element_ids.push_back(tmp_element_ids_2);
      }
      selected_element_ids.push_back(tmp_element_ids);
    }
  }

  return selected_element_ids;
}

std::vector<std::vector<std::vector<int>>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_draw_element_ids(int load_id)
{
  std::vector<std::vector<std::vector<int>>> element_ids = this->get_element_ids(load_id);
  //element_ids[0] order by node
  //element_ids[0][0] order by radius
  //element_ids[0][0][0] element ids

  for (size_t i = 0; i < element_ids.size(); i++)
  {
    for (size_t ii = 0; ii < element_ids[i].size()-1; ii++)
    {
      sort(begin(element_ids[i][ii]), end(element_ids[i][ii]));
      // loop over all nodes
      for (size_t ni = 0; ni < element_ids.size(); ni++)
      {
          for (size_t iii = ii+1; iii < element_ids[ni].size(); iii++) //loop over all bigger radius
          { 
          element_ids[ni][iii].erase( remove_if( begin(element_ids[ni][iii]),end(element_ids[ni][iii]),
              [&](auto x){return binary_search(begin(element_ids[i][ii]),end(element_ids[i][ii]),x);}), end(element_ids[ni][iii]) );
          }
      }
    }
  }

  return element_ids;
}

std::vector<std::vector<double>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_times(int load_id)
{
  std::vector<std::vector<double>> times;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    
    int time_data_id = this->get_time_data_id_from_time_id(loads_data[load_data_id][7]);
    
    double delta_t = (ccx_iface->string_scientific_to_double(time_data[time_data_id][2]) - ccx_iface->string_scientific_to_double(time_data[time_data_id][1]))/double(node_ids.size());
    
    double t_begin = ccx_iface->string_scientific_to_double(time_data[time_data_id][1]);
    double t_end = t_begin + delta_t;


    for (size_t i = 0; i < node_ids.size(); i++)
    {
      std::vector<double> time;

      time.push_back(t_begin);
      time.push_back(t_end);

      //std::string log = "t_begin " + std::to_string(t_begin) + " t_end " + std::to_string(t_end) +"\n";
      //PRINT_INFO("%s", log.c_str());

      times.push_back(time);

      t_begin = t_end;
      t_end = t_end + delta_t;
    }
  }

  return times;
}

std::vector<std::vector<double>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_radius(int load_id)
{
  std::vector<std::vector<double>> node_radius;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    std::vector<int> radius_data_ids = this->get_radius_data_ids_from_radius_id(loads_data[load_data_id][8]);

    for (size_t i = 0; i < node_ids.size(); i++)
    {
      std::vector<double> radius;

      for (size_t ii = 0; ii < radius_data_ids.size(); ii++)
      {
        radius.push_back(radius_data[radius_data_ids[ii]][1]);
      }
      node_radius.push_back(radius);
    }
  }

  return node_radius;
}

std::vector<std::vector<double>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_depth(int load_id)
{
  std::vector<std::vector<double>> node_depth;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    std::vector<int> depth_data_ids = this->get_depth_data_ids_from_depth_id(loads_data[load_data_id][9]);

    for (size_t i = 0; i < node_ids.size(); i++)
    {
      std::vector<double> depth;

      for (size_t ii = 0; ii < depth_data_ids.size(); ii++)
      {
        depth.push_back(depth_data[depth_data_ids[ii]][1]);
      }
      node_depth.push_back(depth);
    }
  }

  return node_depth;
}

std::vector<std::vector<double>> CoreLoadsTrajectoryBodyHeatfluxSphere::get_magnitude(int load_id)
{
  std::vector<std::vector<double>> node_magnitude;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    std::vector<int> magnitude_data_ids = this->get_magnitude_data_ids_from_magnitude_id(loads_data[load_data_id][8]);

    for (size_t i = 0; i < node_ids.size(); i++)
    {
      std::vector<double> magnitude;

      for (size_t ii = 0; ii < magnitude_data_ids.size(); ii++)
      {
        magnitude.push_back(magnitude_data[magnitude_data_ids[ii]][1]);
      }
      node_magnitude.push_back(magnitude);
    }
  }

  return node_magnitude;
}

bool CoreLoadsTrajectoryBodyHeatfluxSphere::prepare_export()
{
  StopWatch watch;
  watch.tick("prepare trajectory bodyheatfluxsphere start");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    std::vector<int> node_ids;
    node_ids = this->get_node_ids(loads_data[i][0]);
    std::vector<std::vector<std::vector<int>>> element_ids;
    element_ids = this->get_element_ids(loads_data[i][0]);    
    std::vector<std::vector<double>> times;
    times = this->get_times(loads_data[i][0]);
    std::vector<std::vector<double>> magnitude;
    magnitude = this->get_magnitude(loads_data[i][0]);
    std::vector<std::vector<double>> radius;
    radius = this->get_radius(loads_data[i][0]);
    std::vector<std::vector<double>> depth;
    depth = this->get_radius(loads_data[i][0]);
    int name_data_id = this->get_name_data_id_from_name_id(loads_data[i][9]);
    std::string name = this->name_data[name_data_id][1];
    std::vector<std::vector<std::vector<double>>> amplitude_times;
    std::vector<std::vector<std::vector<double>>> amplitude_magnitudes;
    int last_id_amplitude = 0;
    int last_id_bodyheatflux = 0;
    std::vector<int> bodyheatflux_amplitude; //stores the amplitude id for use in heatflux
    std::vector<int> bodyheatflux; //stores the heatflux ids from the current trajectory

    //check if every vector has the same size, if not, something went wrong
    if ((node_ids.size() != element_ids.size()) && (element_ids.size() != times.size()) && (times.size() != magnitude.size()) && (magnitude.size() != radius.size()) && (depth.size() != radius.size()))
    {
      std::string log = "Something went wrong with preparing export data for TRAJECTORY ID " + std::to_string(loads_data[i][0]) + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // check for overlapping faces and prepare face_ids,times,magnitude according to it
    // sorting for faster search
    std::vector<int> tmp_element_ids;
    std::vector<std::vector<double>> tmp_times;
    std::vector<double> tmp_magnitude;

    for (size_t ii = 0; ii < element_ids.size(); ii++) //loop over nodes
    {
      for (size_t iii = 0; iii < element_ids[ii].size(); iii++) // loop over radius
      {
        for (size_t iv = 0; iv < element_ids[ii][iii].size(); iv++) // loop over element
        {
          tmp_element_ids.push_back(element_ids[ii][iii][iv]);
          tmp_times.push_back(times[ii]);
          tmp_magnitude.push_back(magnitude[ii][iii]);
        }
      }
    }

    /*
    for (size_t ii = 0; ii < tmp_face_ids.size(); ii++)
    {
      std::string log = "unsorted[ii] " + std::to_string(tmp_face_ids[ii]) + " " + std::to_string(tmp_times[ii][0]) + " " + std::to_string(tmp_times[ii][1]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */

    auto p = sort_permutation(tmp_element_ids);
    this->apply_permutation(tmp_element_ids, p);
    this->apply_permutation(tmp_times, p);
    this->apply_permutation(tmp_magnitude, p);
    
    /*
    for (size_t ii = 0; ii < tmp_face_ids.size(); ii++)
    {
      std::string log = "sorted[ii] " + std::to_string(tmp_face_ids[ii]) + " " + std::to_string(tmp_times[ii][0]) + " " + std::to_string(tmp_times[ii][1]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */

    element_ids.clear();
    times.clear();
    magnitude.clear();

    int element_id;
    std::vector<std::vector<int>> trajectory_element_ids;
    std::vector<std::vector<double>> element_times; // to get all times when the element is active
    std::vector<std::vector<double>> element_magnitudes; //saves the magnitude according to the element times
    for (size_t ii = 0; ii < tmp_element_ids.size(); ii++)
    {
      if (ii==0)
      {
        element_id = tmp_element_ids[ii];
        element_times.push_back(tmp_times[ii]);
        element_magnitudes.push_back({0,tmp_magnitude[ii]});
      }else{
        if (element_id != tmp_element_ids[ii]) //check if its a new element
        {
          trajectory_element_ids.push_back({element_id});
          amplitude_times.push_back(element_times);
          amplitude_magnitudes.push_back(element_magnitudes);
          element_times.clear();
          element_magnitudes.clear();
          element_id = tmp_element_ids[ii];
          element_times.push_back(tmp_times[ii]);
          element_magnitudes.push_back({0,tmp_magnitude[ii]});
        }else{
          element_times.push_back(tmp_times[ii]);
          element_magnitudes.push_back({0,tmp_magnitude[ii]});
        }
      }
      if (ii==tmp_element_ids.size()-1)
      {        
        trajectory_element_ids.push_back({element_id});
        amplitude_times.push_back(element_times);
        amplitude_magnitudes.push_back(element_magnitudes);
      }
    }

    /*
    for (size_t ii = 0; ii < trajectory_element_ids.size(); ii++)
    {
      std::string log = "filtered[ii] " + std::to_string(trajectory_element_ids[ii][0]) + " " + std::to_string(times[ii][0]) + " " + std::to_string(times[ii][1]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " filtered " + std::to_string(trajectory_element_ids.size()) +  " elements");
    //block core update
    ccx_iface->set_block_core_update(true);    
    // prepare amplitudes
    std::vector<int> amplitude_ids = ccx_iface->parser("amplitude","all");
    if (amplitude_ids.size()>0)
    {
      last_id_amplitude = amplitude_ids[amplitude_ids.size()-1] + 1;
    }else{
      last_id_amplitude = 1;
    }
    for (size_t ii = 0; ii < trajectory_element_ids.size(); ii++)
    {
      if (trajectory_element_ids[ii].size()>0)
      {
        //reorder times and magnitudes
        std::vector<double> temp;
        for (size_t iii = 0; iii < amplitude_times[ii].size(); iii++)
        {
          temp.push_back(amplitude_times[ii][iii][0]);
        }
        auto p = sort_permutation(temp);
        this->apply_permutation(amplitude_times[ii], p);
        this->apply_permutation(amplitude_magnitudes[ii], p);
        
        std::string amplitude = "";
        bool zero_magnitude = false;
        
        for (size_t iii = 0; iii < amplitude_times[ii].size(); iii++)
        {
        //std::string log = std::to_string(iii) +" "+ std::to_string(amplitude_times[ii][iii][0]) + " " +  std::to_string(amplitude_times[ii][iii][1]) +" "+ std::to_string(amplitude_magnitudes[ii][iii][0]) + " " +  std::to_string(amplitude_magnitudes[ii][iii][1]) + "\n";
        //PRINT_INFO("%s", log.c_str());
          if (iii==0)
          {
            if (amplitude_times[ii][iii][0] == 0.)
            {
              amplitude = "0 0 0 " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ";
            }else{
              amplitude = "0 0 ";
              amplitude.append(std::to_string(amplitude_times[ii][iii][0]) + " 0 ");
              amplitude.append(std::to_string(amplitude_times[ii][iii][0]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ");
            }
            zero_magnitude = false;
          }
          //check if next time is the same
          if (iii<amplitude_times[ii].size()-1)
          {
            if (zero_magnitude)
            {
              amplitude.append(std::to_string(amplitude_times[ii][iii][0]) + " 0 ");
              amplitude.append(std::to_string(amplitude_times[ii][iii][0]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ");
              zero_magnitude = false;
            }
            if (pow((amplitude_times[ii][iii][1]-amplitude_times[ii][iii+1][0]),2) < 1e-12)
            {
              //don't stop magnitude but adjust magnitude value to next value
              amplitude.append(std::to_string(amplitude_times[ii][iii][1]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ");
              amplitude.append(std::to_string(amplitude_times[ii][iii][1]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii+1][1]) + " ");
            }else{
              amplitude.append(std::to_string(amplitude_times[ii][iii][1]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ");
              amplitude.append(std::to_string(amplitude_times[ii][iii][1]) + " 0 ");
              zero_magnitude = true;
            }
          }
          
          if (iii==amplitude_times[ii].size()-1)
          {
            if (zero_magnitude)
            {
              amplitude.append(std::to_string(amplitude_times[ii][iii][0]) + " 0 ");
              amplitude.append(std::to_string(amplitude_times[ii][iii][0]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ");
              zero_magnitude = false;
            }
            amplitude.append(std::to_string(amplitude_times[ii][iii][1]) + " " + ccx_iface->to_string_scientific(amplitude_magnitudes[ii][iii][1]) + " ");
            amplitude.append(std::to_string(amplitude_times[ii][iii][1]) + " 0 ");
          } 
        }
        //std::string log = std::to_string(trajectory_element_ids[ii][0]) + " " + std::to_string(amplitude_times[ii].size()) + " " +  amplitude + "\n";
        //PRINT_INFO("%s", log.c_str());

        ccx_iface->silent_cmd("ccx create amplitude name \"Trajectory_" + std::to_string(loads_data[i][0]) + "_" + name + "_"  + std::to_string(trajectory_element_ids[ii][0]) + "\" time_amplitude " + amplitude);
        prepared_amplitudes.push_back(last_id_amplitude);
        bodyheatflux_amplitude.push_back(last_id_amplitude);
        last_id_amplitude = last_id_amplitude + 1;
      }
    }
    
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " amplitudes");
    
    // prepare bodyheatflux
    std::vector<int> bodyheatflux_ids = ccx_iface->get_loadsbodyheatflux_ids();
    if (bodyheatflux_ids.size()>0)
    {
      last_id_bodyheatflux = bodyheatflux_ids[bodyheatflux_ids.size()-1] + 1;
    }else{
      last_id_bodyheatflux = 1;
    }
    int link_id = 0;
    //block core update
    ccx_iface->set_block_core_update(true);
    for (size_t ii = 0; ii < trajectory_element_ids.size(); ii++)
    {
      if (trajectory_element_ids[ii].size()>0)
      {
        ccx_iface->silent_cmd("ccx create bodyheatflux 1 element " + std::to_string(trajectory_element_ids[ii][0]) + " amplitude " + std::to_string(bodyheatflux_amplitude[link_id]));
        prepared_bodyheatflux.push_back(last_id_bodyheatflux);
        bodyheatflux.push_back(last_id_bodyheatflux);
        link_id = link_id + 1;
        last_id_bodyheatflux = last_id_bodyheatflux + 1;
      }
    }

    //core update
    ccx_iface->set_block_core_update(false);
    ccx_iface->core_update();
    //block core update
    ccx_iface->set_block_core_update(true);
    
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " bodyheatflux");
    //link bodyheatflux to steps
    std::vector<std::vector<std::string>> steps_tree = ccx_iface->get_steps_tree_data();
    for (size_t ii = 0; ii < steps_tree.size(); ii++)
    {
      std::vector<std::vector<std::string>> trajectory_tree = ccx_iface->get_steps_loadstrajectory_tree_data(std::stoi(steps_tree[ii][0]));
      for (size_t iii = 0; iii < trajectory_tree.size(); iii++)
      {
        if (trajectory_tree[iii][0] == std::to_string(loads_data[i][0]))
        {
          std::string ids = "";
          for (size_t iv= 0; iv < bodyheatflux.size(); iv++)
          {
            ids.append(std::to_string(bodyheatflux[iv]) + " ");
            prepared_step_bodyheatflux.push_back({std::stoi(steps_tree[ii][0]),bodyheatflux[iv]});
          }
          ccx_iface->silent_cmd("ccx step " + steps_tree[ii][0] + " add load bodyheatflux " + ids);
        }
      }
    }
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " steps");
    //resume core update
    ccx_iface->set_block_core_update(false);
    
  }
  watch.tick("prepare trajectory end");
  
  return true;
}


bool CoreLoadsTrajectoryBodyHeatfluxSphere::clean_export()
{
  std::string ids;
  StopWatch watch;
  if (prepared_amplitudes.size()==0)
  {
    return true;
  }
  
  watch.tick("clean trajectory start");

  //block core update
  ccx_iface->set_block_core_update(true);
  ids = "";
  int current_step_id=0;
  for (size_t i = 0; i < prepared_step_bodyheatflux.size(); i++)
  {
    if (i==0)
    {
      current_step_id = prepared_step_bodyheatflux[i][0];
    }
    if (current_step_id == prepared_step_bodyheatflux[i][0])
    {
      ccx_iface->silent_cmd("ccx step " + std::to_string(current_step_id) + " remove load bodyheatflux " + ids);
      ids="";
      ids.append(std::to_string(prepared_step_bodyheatflux[i][1]) + " ");
      current_step_id = prepared_step_bodyheatflux[i][0];
    }else{
      ids.append(std::to_string(prepared_step_bodyheatflux[i][1]) + " ");
    }
    if (i==prepared_step_bodyheatflux.size()-1)
    {
      ccx_iface->silent_cmd("ccx step " + std::to_string(current_step_id) + " remove load bodyheatflux " + ids);
    }
  }
  watch.tick("clean trajectory step links");

  ids = "";
  for (size_t i = 0; i < prepared_bodyheatflux.size(); i++)
  {
    ids.append(std::to_string(prepared_bodyheatflux[i]) + " ");
  }
  ccx_iface->silent_cmd("ccx delete bodyheatflux " + ids);
  watch.tick("clean trajectory bodyheatfluxsphere");

  /*
  ids = "";
  for (size_t i = 0; i < prepared_amplitudes.size(); i++)
  {
    ids.append(std::to_string(prepared_amplitudes[i]) + " ");
  }
  ccx_iface->silent_cmd("ccx delete amplitude " + ids);
  */
  ccx_iface->delete_amplitudes(prepared_amplitudes);
  watch.tick("clean trajectory amplitudes");

  //resume core update
  ccx_iface->set_block_core_update(false);
  ccx_iface->core_update();
  
  prepared_amplitudes.clear();
  prepared_bodyheatflux.clear();
  prepared_step_bodyheatflux.clear();

  watch.tick("clean trajectory end");
  return true;
}

std::string CoreLoadsTrajectoryBodyHeatfluxSphere::get_load_export(int load_id)
{
  
  int load_data_id;
  int sub_data_id;
  std::string str_temp = "**Trajectory " + std::to_string(load_id) + "\n";
  
  /*
  load_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data[load_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (loads_data[load_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (loads_data[load_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_iface->get_amplitude_name(loads_data[load_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[load_data_id][3]);
  if (time_delay_data[sub_data_id][1]!="")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  str_temp.append("\n");
  
  // second line
  str_temp.append(ccx_iface->get_block_name(loads_data[load_data_id][4]) + ",CENTRIF,");
  
  sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[load_data_id][6]);
  str_temp.append(magnitude_data[sub_data_id][1] + ",");
  
  sub_data_id = get_coordinate_data_id_from_coordinate_id(loads_data[load_data_id][7]);
  str_temp.append(coordinate_data[sub_data_id][1] + "," + coordinate_data[sub_data_id][2] + "," + coordinate_data[sub_data_id][3] + ",");

  sub_data_id = get_direction_data_id_from_direction_id(loads_data[load_data_id][5]);
  str_temp.append(direction_data[sub_data_id][1] + "," + direction_data[sub_data_id][2] + "," + direction_data[sub_data_id][3]);
*/
  return str_temp;
}

std::string CoreLoadsTrajectoryBodyHeatfluxSphere::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsTrajectoryBodyHeatfluxSphere loads_data: \n";
  str_return.append("load_id, OP MODE, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id, radius_id, name_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6]) + " " + std::to_string(loads_data[i][7]) + " " + std::to_string(loads_data[i][8]) + " " + std::to_string(loads_data[i][9]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere time_data: \n");
  str_return.append("time_id, t_begin, t_end \n");

  for (size_t i = 0; i < time_data.size(); i++)
  {
    str_return.append(time_data[i][0] + " " + time_data[i][1] + " " + time_data[i][2] + " \n");
  }

  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere direction_data: \n");
  str_return.append("direction_id, x, y, z \n");

  for (size_t i = 0; i < direction_data.size(); i++)
  {
    str_return.append(direction_data[i][0] + " " + direction_data[i][1] + " " + direction_data[i][2] + " " + direction_data[i][3] + " \n");
  }
  
  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value \n");

  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    str_return.append(std::to_string(magnitude_data[i][0]) + " " + std::to_string(magnitude_data[i][1]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere radius_data: \n");
  str_return.append("radius_id, radius_value \n");

  for (size_t i = 0; i < radius_data.size(); i++)
  {
    str_return.append(std::to_string(radius_data[i][0]) + " " + std::to_string(radius_data[i][1]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere depth_data: \n");
  str_return.append("depth_id, depth_value \n");

  for (size_t i = 0; i < depth_data.size(); i++)
  {
    str_return.append(std::to_string(depth_data[i][0]) + " " + std::to_string(depth_data[i][1]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere fire_ray_surface_data: \n");
  str_return.append("fire_ray_surface_id, surface_id \n");

  for (size_t i = 0; i < fire_ray_surface_data.size(); i++)
  {
    str_return.append(std::to_string(fire_ray_surface_data[i][0]) + " " + std::to_string(fire_ray_surface_data[i][1]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectoryBodyHeatfluxSphere name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }


  return str_return;
}


//sorting of vectors
template <typename T> 
std::vector<std::size_t> CoreLoadsTrajectoryBodyHeatfluxSphere::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreLoadsTrajectoryBodyHeatfluxSphere::apply_permutation(
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