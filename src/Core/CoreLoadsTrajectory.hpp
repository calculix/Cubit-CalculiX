#ifndef CORELOADSTRAJECTORY_HPP
#define CORELOADSTRAJECTORY_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreLoadsTrajectory
{

public:
  CoreLoadsTrajectory();
  ~CoreLoadsTrajectory();

  std::vector<std::vector<int>> loads_data; // used to store the connection between a trajectory and its possible options
  // loads_data[0][0] load_id
  // loads_data[0][1] OP mode: 0 for OP=MOD | 1  for OP=NEW
  // loads_data[0][2] curve_id
  // loads_data[0][3] fire_ray_surface_id
  // loads_data[0][5] direction_id
  // loads_data[0][6] magnitude_id
  // loads_data[0][6] time_id
  
  std::vector<std::vector<std::string>> time_data;
  // time_delay_data[0][0] time_id
  // time_delay_data[0][1] t_begin
  // time_delay_data[0][2] t_end
  // time_delay_data[0][3] t_delta

  std::vector<std::vector<std::string>> direction_data;
  // direction_data[0][0] direction_id
  // direction_data[0][1] x
  // direction_data[0][2] y
  // direction_data[0][3] z

  std::vector<std::vector<std::string>> magnitude_data;
  // magnitude_data[0][0] magnitude_id
  // magnitude_data[0][1] magnitude

  std::vector<std::vector<std::string>> fire_ray_surface_data;
  // magnitude_data[0][0] fire_ray_surface_id
  // magnitude_data[0][1] surface_id
  
  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the blocks
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_load(std::vector<std::string> options); // adds new load
  bool add_load(int load_id, int op_mode, int curve_id, int fire_ray_surface_id, int direction_id, int magnitude_id, int time_id); // adds new load to loads_data
  bool modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a load
  bool delete_load(int load_id); // deletes load from loads_data
  bool add_time(std::string time_id, std::string t_begin, std::string t_end, std::string t_delta); // adds new time to time_data
  bool add_direction(std::string direction_id, std::string x, std::string y, std::string z); // adds new direction to direction_data
  bool add_magnitude(std::string magnitude_id, std::string magnitude_value); // adds new magnitude to magnitude_data
  bool add_fire_ray_surface(std::string fire_ray_surface_id, std::string surface_id); // adds new surface id to fire_ray_surface_data
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  int  get_time_data_id_from_time_id(int time_id); // searches for the time_delay_id in the time_delay_data and returns the indices or -1 if it fails
  int  get_magnitude_data_id_from_magnitude_id(int magnitude_id); // searches for the magnitude_id in the magnitude_data and returns the indices or -1 if it fails
  int  get_direction_data_id_from_direction_id(int direction_id); // searches for the time_delay_id in the direction_data and returns the indices or -1 if it fails
  std::vector<int> get_fire_ray_surface_data_id_from_fire_ray_surface_id(int fire_ray_surface_id); // searches for the fire_ray_surface_id in the fire_ray_surface_data and returns an empty vector if it fails
  std::string get_load_export(int load_id); // gets the optional parameters for the export
  std::string print_data(); // prints out the loads_data
 
  CalculiXCoreInterface *ccx_iface;
};

#endif // CORELOADSTRAJECTORY_HPP