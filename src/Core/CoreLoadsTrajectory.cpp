#include "CoreLoadsTrajectory.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <algorithm>
#include "StopWatch.hpp"

CoreLoadsTrajectory::CoreLoadsTrajectory()
{}

CoreLoadsTrajectory::~CoreLoadsTrajectory()
{}

bool CoreLoadsTrajectory::init()
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

bool CoreLoadsTrajectory::update()
{ 
  return true;
}

bool CoreLoadsTrajectory::reset()
{
  loads_data.clear();
  fire_ray_surface_data.clear();
  time_data.clear();
  direction_data.clear();
  magnitude_data.clear();
  radius_data.clear();
  init();
  return true;
}

bool CoreLoadsTrajectory::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsTrajectory::create_load(std::vector<std::string> options, std::vector<int> options2)
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
    sub_last = int(magnitude_data.size()) - 1;
    sub_id = std::stoi(magnitude_data[sub_last][0]) + 1;
  }
  magnitude_id = sub_id;
  this->add_magnitude(std::to_string(sub_id), options[6]);

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
  this->add_time(std::to_string(sub_id), options[7], options[8]);
  
  // radius
  if (radius_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(radius_data.size()) - 1;
    sub_id = std::stoi(radius_data[sub_last][0]) + 1;
  }
  radius_id = sub_id;
  this->add_radius(std::to_string(sub_id), options[9]);

  this->add_load(load_id, op_mode, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id, radius_id);
  return true;
}

bool CoreLoadsTrajectory::add_load(int load_id, int op_mode, int curve_id, int vertex_id, int fire_ray_surface_id, int direction_id, int magnitude_id, int time_id, int radius_id)
{
  std::vector<int> v = {load_id, op_mode, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id, radius_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2)
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
      sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
      magnitude_data[sub_data_id][1] = options[7];
    }
    // time
    if (options_marker[8]==1)
    {
      sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
      time_data[sub_data_id][1] = options[8];
    }
    if (options_marker[9]==1)
    {
      sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
      time_data[sub_data_id][2] = options[9];
    }
    // radius
    if (options_marker[10]==1)
    {
      sub_data_id = get_radius_data_id_from_radius_id(loads_data[loads_data_id][8]);
      radius_data[sub_data_id][1] = options[10];
    }
    return true;
  }
}

bool CoreLoadsTrajectory::delete_load(int load_id)
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
    sub_data_id = get_radius_data_id_from_radius_id(loads_data[loads_data_id][8]);
    if (sub_data_id != -1){
      radius_data.erase(radius_data.begin() + sub_data_id);
    }
    // direction
    sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      direction_data.erase(direction_data.begin() + sub_data_id);
    }
    // magnitude
    sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
    if (sub_data_id != -1){
      magnitude_data.erase(magnitude_data.begin() + sub_data_id);
    }
    // fire ray surfaces
    sub_data_ids = get_fire_ray_surface_data_ids_from_fire_ray_surface_id(loads_data[loads_data_id][4]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      fire_ray_surface_data.erase(fire_ray_surface_data.begin() + sub_data_ids[i-1]);
    }
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsTrajectory::add_time(std::string time_id, std::string t_begin, std::string t_end)
{
  std::vector<std::string> v = {time_id, t_begin, t_end};
      
  time_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_radius(std::string radius_id, std::string radius)
{
  std::vector<std::string> v = {radius_id, radius};
      
  radius_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_direction(std::string direction_id, std::string x, std::string y, std::string z)
{
  std::vector<std::string> v = {direction_id, x, y, z};
      
  direction_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_magnitude(std::string magnitude_id, std::string magnitude_value)
{
  std::vector<std::string> v = {magnitude_id, magnitude_value};
      
  magnitude_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_fire_ray_surface(int fire_ray_surface_id, int surface_id)
{
  std::vector<int> v = {fire_ray_surface_id, surface_id};
      
  fire_ray_surface_data.push_back(v);

  return true;
}

int CoreLoadsTrajectory::get_loads_data_id_from_load_id(int load_id)
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

int CoreLoadsTrajectory::get_time_data_id_from_time_id(int time_id)
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

int CoreLoadsTrajectory::get_radius_data_id_from_radius_id(int radius_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < radius_data.size(); i++)
  {
    if (radius_data[i][0]==std::to_string(radius_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsTrajectory::get_direction_data_id_from_direction_id(int direction_id)
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

int CoreLoadsTrajectory::get_magnitude_data_id_from_magnitude_id(int magnitude_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    if (magnitude_data[i][0]==std::to_string(magnitude_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectory::get_fire_ray_surface_data_ids_from_fire_ray_surface_id(int fire_surface_id)
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

std::vector<int> CoreLoadsTrajectory::get_fire_ray_surface_ids_from_fire_ray_surface_id(int fire_surface_id)
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

std::vector<int> CoreLoadsTrajectory::get_node_ids(int load_id)
{
  std::vector<int> node_ids;
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
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

  return node_ids;
}

std::vector<int> CoreLoadsTrajectory::get_edge_ids(int load_id)
{
  std::vector<int> edge_ids;
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    edge_ids = CubitInterface::get_curve_edges(loads_data[load_data_id][2]);
  }

  return edge_ids;
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_hit_coordinates(int load_id)
{
  std::vector<std::vector<double>> hit_coordinates;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    std::vector<int> surface_ids = this->get_fire_ray_surface_ids_from_fire_ray_surface_id(loads_data[load_data_id][4]);
    
    int direction_data_id = this->get_direction_data_id_from_direction_id(loads_data[load_data_id][5]);
    int radius_data_id = this->get_radius_data_id_from_radius_id(loads_data[load_data_id][8]);
    double radius = 0;
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

std::vector<std::vector<int>> CoreLoadsTrajectory::get_face_ids(int load_id)
{
  std::vector<std::vector<int>> selected_face_ids;
  
  int load_data_id = this->get_loads_data_id_from_load_id(load_id);

  if (load_data_id!=-1)
  {
    std::vector<int> node_ids = this->get_node_ids(load_id);
    std::vector<int> surface_ids = this->get_fire_ray_surface_ids_from_fire_ray_surface_id(loads_data[load_data_id][4]);
    std::vector<int> face_ids;
    for (size_t i = 0; i < surface_ids.size(); i++)
    {
      std::vector<int> tmp_face_ids = CubitInterface::parse_cubit_list("face","in surface " + std::to_string(surface_ids[i]));
      for (size_t ii = 0; ii < tmp_face_ids.size(); ii++)
      {
        face_ids.push_back(tmp_face_ids[ii]);
      }
    }
    
    int direction_data_id = this->get_direction_data_id_from_direction_id(loads_data[load_data_id][5]);
    int radius_data_id = this->get_radius_data_id_from_radius_id(loads_data[load_data_id][8]);
    double radius = 0;
    std::array<double, 3> direction;

    direction[0] = ccx_iface->string_scientific_to_double(direction_data[direction_data_id][1]);
    direction[1] = ccx_iface->string_scientific_to_double(direction_data[direction_data_id][2]);
    direction[2] = ccx_iface->string_scientific_to_double(direction_data[direction_data_id][3]); 

    for (size_t i = 0; i < node_ids.size(); i++)
    {
      std::vector<int> tmp_selected_face_ids;
      std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
      std::pair<std::vector<std::array<double, 3>>, std::vector<int>> hit = CubitInterface::fire_ray(coord, direction, "surface", surface_ids, 1, radius);

      if (radius_data[radius_data_id][1]!="")
      {
        radius = ccx_iface->string_scientific_to_double(radius_data[radius_data_id][1]);
      }
      
      if (hit.first.size()>0)
      {
        for (size_t ii = 0; ii < face_ids.size(); ii++)
        {
          std::array<double,3> center_point;
          center_point = CubitInterface::get_center_point("face", face_ids[ii]);

          double distance = sqrt(pow(center_point[0]-hit.first[0][0],2)+pow(center_point[1]-hit.first[0][1],2)+pow(center_point[2]-hit.first[0][2],2));

          if (distance <= radius)
          {
            tmp_selected_face_ids.push_back(face_ids[ii]);
          }
        }
      }
      selected_face_ids.push_back(tmp_selected_face_ids);
    }
  }

  return selected_face_ids;
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_times(int load_id)
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

bool CoreLoadsTrajectory::prepare_export()
{
  StopWatch watch;
  watch.tick("prepare trajectory start");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    std::vector<int> node_ids;
    node_ids = this->get_node_ids(loads_data[i][0]);
    std::vector<std::vector<int>> face_ids;
    face_ids = this->get_face_ids(loads_data[i][0]);    
    std::vector<std::vector<double>> times;
    times = this->get_times(loads_data[i][0]);
    int last_id_sideset = 0;
    int last_id_amplitude = 0;
    int last_id_heatflux = 0;
    int magnitude_data_id = this->get_magnitude_data_id_from_magnitude_id(loads_data[i][6]);
    std::vector<int> heatflux_sidesets; //stores the sideset id for use in heatflux
    std::vector<int> heatflux_amplitude; //stores the amplitude id for use in heatflux
    std::vector<int> heatflux; //stores the heatflux ids from the current trajectory

    //check if every vector has the same size, if not, something went wrong
    if ((node_ids.size() != face_ids.size()) && (face_ids.size() != times.size()))
    {
      std::string log = "Something went wrong with preparing export data for TRAJECTORY ID " + std::to_string(loads_data[i][0]) + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    //check for overlapping faces and prepare face_ids and times according to it
    // sorting for faster search
    std::vector<int> tmp_face_ids;
    std::vector<std::vector<double>> tmp_times;

    for (size_t ii = 0; ii < face_ids.size(); ii++)
    {
      for (size_t iii = 0; iii < face_ids[ii].size(); iii++)
      {
        tmp_face_ids.push_back(face_ids[ii][iii]);
        tmp_times.push_back(times[ii]);
      }
    }

    /*
    for (size_t ii = 0; ii < tmp_face_ids.size(); ii++)
    {
      std::string log = "unsorted[ii] " + std::to_string(tmp_face_ids[ii]) + " " + std::to_string(tmp_times[ii][0]) + " " + std::to_string(tmp_times[ii][1]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */

    auto p = sort_permutation(tmp_face_ids);
    this->apply_permutation(tmp_face_ids, p);
    this->apply_permutation(tmp_times, p);
    
    /*
    for (size_t ii = 0; ii < tmp_face_ids.size(); ii++)
    {
      std::string log = "sorted[ii] " + std::to_string(tmp_face_ids[ii]) + " " + std::to_string(tmp_times[ii][0]) + " " + std::to_string(tmp_times[ii][1]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */

    face_ids.clear();
    times.clear();

    int face_id;
    std::vector<double> face_times;
    for (size_t ii = 0; ii < tmp_face_ids.size(); ii++)
    {
      if (ii==0)
      {
        face_id = tmp_face_ids[ii];
        face_times = tmp_times[ii];
      }else{
        if (face_id != tmp_face_ids[ii]) //check if its a new face
        {
          face_ids.push_back({face_id});
          times.push_back(face_times);
          face_id = tmp_face_ids[ii];
          face_times = tmp_times[ii];
        }else{ // check if times needs to be adjusted
          if (face_times[0] > tmp_times[ii][0])
          {
            face_times[0] = tmp_times[ii][0];
          }
          if (face_times[1] < tmp_times[ii][1])
          {
            face_times[1] = tmp_times[ii][1];
          }
        }
      }
      if (ii==tmp_face_ids.size()-1)
      {
        face_ids.push_back({face_id});
        times.push_back(face_times);
      }
    }

    /*
    for (size_t ii = 0; ii < face_ids.size(); ii++)
    {
      std::string log = "filtered[ii] " + std::to_string(face_ids[ii][0]) + " " + std::to_string(times[ii][0]) + " " + std::to_string(times[ii][1]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " filtered " + std::to_string(face_ids.size()) +  " faces");
    //prepare sidesets
    std::vector<int> sideset_ids = CubitInterface::parse_cubit_list("sideset","all");
    if (sideset_ids.size()>0)
    {
      last_id_sideset = sideset_ids[sideset_ids.size()-1];
    }else{
      last_id_sideset = 1;
    }
    for (size_t ii = 0; ii < face_ids.size(); ii++)
    {
      if (face_ids[ii].size()>0)
      {
        std::string face = "";
        for (size_t iii = 0; iii < face_ids[ii].size(); iii++)
        {
          face.append(std::to_string(face_ids[ii][iii]) + " ");
        }
        ccx_iface->silent_cmd("sideset " + std::to_string(last_id_sideset) + " add face " + face);
        ccx_iface->silent_cmd("sideset " + std::to_string(last_id_sideset) + " name \"Trajectory_" + std::to_string(loads_data[i][0]) + "_" + std::to_string(face_ids[ii][0]) + "_" + std::to_string(times[ii][0])+ "_" + std::to_string(times[ii][1]) + "\"");
        prepared_sidesets.push_back(last_id_sideset);
        heatflux_sidesets.push_back(last_id_sideset);
        last_id_sideset = last_id_sideset + 1;
      }
    }
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " sidesets");
    // prepare amplitudes
    std::vector<int> amplitude_ids = ccx_iface->parser("amplitude","all");
    if (amplitude_ids.size()>0)
    {
      last_id_amplitude = amplitude_ids[amplitude_ids.size()-1];
    }else{
      last_id_amplitude = 1;
    }
    for (size_t ii = 0; ii < face_ids.size(); ii++)
    {
      if (face_ids[ii].size()>0)
      {
        std::string amplitude = "0 0 ";
        amplitude.append(std::to_string(times[ii][0]) + " 0 ");
        amplitude.append(std::to_string(times[ii][0]) + " 1 ");
        amplitude.append(std::to_string(times[ii][1]) + " 1 ");
        amplitude.append(std::to_string(times[ii][1]) + " 0 ");

        ccx_iface->silent_cmd("ccx create amplitude name \"Trajectory_" + std::to_string(loads_data[i][0]) + "_" + std::to_string(face_ids[ii][0]) + "_" + std::to_string(times[ii][0])+ "_" + std::to_string(times[ii][1]) + "\" time_amplitude " + amplitude);
        prepared_amplitudes.push_back(last_id_amplitude);
        heatflux_amplitude.push_back(last_id_amplitude);
        last_id_amplitude = last_id_amplitude + 1;
      }
    }
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " amplitudes");
    // prepare heatflux
    std::vector<int> heatflux_ids = CubitInterface::parse_cubit_list("heatflux","all");
    if (heatflux_ids.size()>0)
    {
      last_id_heatflux = heatflux_ids[heatflux_ids.size()-1];
    }else{
      last_id_heatflux = 1;
    }
    int link_id = 0;
    for (size_t ii = 0; ii < face_ids.size(); ii++)
    {
      if (face_ids[ii].size()>0)
      {
        ccx_iface->silent_cmd("create heatflux on sideset " + std::to_string(heatflux_sidesets[link_id]) + " value " + magnitude_data[magnitude_data_id][1]);
        //ccx_iface->silent_cmd("modify heatflux " + std::to_string(last_id_heatflux) + " name \"Trajectory_" + std::to_string(loads_data[i][0]) + "_" + std::to_string(face_ids[ii][0])+ "_" + std::to_string(times[ii][0])+ "_" + std::to_string(times[ii][1]) + "\"");
        ccx_iface->silent_cmd("ccx modify heatflux " + std::to_string(last_id_heatflux) + " amplitude " + std::to_string(heatflux_amplitude[link_id]));
        prepared_heatflux.push_back(last_id_heatflux);
        heatflux.push_back(last_id_heatflux);
        link_id = link_id + 1;
        last_id_heatflux = last_id_heatflux + 1;
      }
    }
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " heatflux");
    //link heatflux to steps
    std::vector<std::vector<std::string>> steps_tree = ccx_iface->get_steps_tree_data();
    for (size_t ii = 0; ii < steps_tree.size(); ii++)
    {
      std::vector<std::vector<std::string>> trajectory_tree = ccx_iface->get_steps_loadstrajectory_tree_data(std::stoi(steps_tree[ii][0]));
      for (size_t iii = 0; iii < trajectory_tree.size(); iii++)
      {
        if (trajectory_tree[iii][0] == std::to_string(loads_data[i][0]))
        {
          std::string ids = "";
          for (size_t iv= 0; iv < heatflux.size(); iv++)
          {
            ids.append(std::to_string(heatflux[iv]) + " ");
            prepared_step_heatflux.push_back({std::stoi(steps_tree[ii][0]),heatflux[iv]});
          }
          ccx_iface->silent_cmd("ccx step " + steps_tree[ii][0] + " add load heatflux " + ids);
        }
      }
    }
    watch.tick("prepare trajectory " + std::to_string(loads_data[i][0]) + " steps");
  }
  watch.tick("prepare trajectory end");
  return true;
}


bool CoreLoadsTrajectory::clean_export()
{
  std::string ids;
  StopWatch watch;
  watch.tick("clean trajectory start");

  ids = "";
  int current_step_id=0;
  for (size_t i = 0; i < prepared_step_heatflux.size(); i++)
  {
    if (i==0)
    {
      current_step_id = prepared_step_heatflux[i][0];
    }
    if (current_step_id == prepared_step_heatflux[i][0])
    {
      ccx_iface->silent_cmd("ccx step " + std::to_string(current_step_id) + " remove load heatflux " + ids);
      ids="";
      ids.append(std::to_string(prepared_step_heatflux[i][1]) + " ");
      current_step_id = prepared_step_heatflux[i][0];
    }else{
      ids.append(std::to_string(prepared_step_heatflux[i][1]) + " ");
    }
    if (i==prepared_step_heatflux.size()-1)
    {
      ccx_iface->silent_cmd("ccx step " + std::to_string(current_step_id) + " remove load heatflux " + ids);
    }
  }
  watch.tick("clean trajectory step links");
  
  ids = "";
  for (size_t i = 0; i < prepared_sidesets.size(); i++)
  {
    ids.append(std::to_string(prepared_sidesets[i]) + " ");
  }
  ccx_iface->silent_cmd("delete sideset " + ids);
  watch.tick("clean trajectory sideset");

  ids = "";
  for (size_t i = 0; i < prepared_heatflux.size(); i++)
  {
    ids.append(std::to_string(prepared_heatflux[i]) + " ");
  }
  ccx_iface->silent_cmd("delete heatflux " + ids);
  watch.tick("clean trajectory heatflux");

  ids = "";
  for (size_t i = 0; i < prepared_amplitudes.size(); i++)
  {
    ids.append(std::to_string(prepared_amplitudes[i]) + " ");
  }
  ccx_iface->silent_cmd("ccx delete amplitude " + ids);
  watch.tick("clean trajectory amplitudes");

  prepared_sidesets.clear();
  prepared_amplitudes.clear();
  prepared_heatflux.clear();
  prepared_step_heatflux.clear();

  watch.tick("clean trajectory end");
  return true;
}

std::string CoreLoadsTrajectory::get_load_export(int load_id)
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

std::string CoreLoadsTrajectory::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsTrajectory loads_data: \n";
  str_return.append("load_id, OP MODE, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6]) + " " + std::to_string(loads_data[i][7]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectory time_data: \n");
  str_return.append("time_id, t_begin, t_end \n");

  for (size_t i = 0; i < time_data.size(); i++)
  {
    str_return.append(time_data[i][0] + " " + time_data[i][1] + " " + time_data[i][2] + " \n");
  }

  str_return.append("\n CoreLoadsTrajectory direction_data: \n");
  str_return.append("direction_id, x, y, z \n");

  for (size_t i = 0; i < direction_data.size(); i++)
  {
    str_return.append(direction_data[i][0] + " " + direction_data[i][1] + " " + direction_data[i][2] + " " + direction_data[i][3] + " \n");
  }
  
  str_return.append("\n CoreLoadsTrajectory magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value \n");

  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    str_return.append(magnitude_data[i][0] + " " + magnitude_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsTrajectory fire_ray_surface_data: \n");
  str_return.append("fire_ray_surface_id, surface_id \n");

  for (size_t i = 0; i < fire_ray_surface_data.size(); i++)
  {
    str_return.append(std::to_string(fire_ray_surface_data[i][0]) + " " + std::to_string(fire_ray_surface_data[i][1]) + " \n");
  }

  return str_return;
}





//sorting of vectors
template <typename T> 
std::vector<std::size_t> CoreLoadsTrajectory::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreLoadsTrajectory::apply_permutation(
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