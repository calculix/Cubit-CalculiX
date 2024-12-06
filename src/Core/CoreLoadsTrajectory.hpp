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
  // loads_data[0][3] vertex_id
  // loads_data[0][4] fire_ray_surface_id
  // loads_data[0][5] direction_id
  // loads_data[0][6] magnitude_id
  // loads_data[0][7] time_id
  // loads_data[0][8] radius_id

  std::vector<std::vector<int>> fire_ray_surface_data;
  // fire_ray_surface_data[0][0] fire_ray_surface_id
  // fire_ray_surface_data[0][1] surface_id

  std::vector<std::vector<std::string>> direction_data;
  // direction_data[0][0] direction_id
  // direction_data[0][1] x
  // direction_data[0][2] y
  // direction_data[0][3] z

  std::vector<std::vector<std::string>> magnitude_data;
  // magnitude_data[0][0] magnitude_id
  // magnitude_data[0][1] magnitude

  std::vector<std::vector<std::string>> time_data;
  // time_data[0][0] time_id
  // time_data[0][1] t_begin
  // time_data[0][2] t_end

  std::vector<std::vector<std::string>> radius_data;
  // time_delay_data[0][0] radius_id
  // time_delay_data[0][1] radius
  
  std::vector<int> prepared_sidesets;
  std::vector<int> prepared_amplitudes;
  std::vector<int> prepared_heatflux;
  std::vector<std::vector<int>> prepared_step_heatflux;

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the blocks
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_load(std::vector<std::string> options, std::vector<int> options2); // adds new load
  bool add_load(int load_id, int op_mode, int curve_id, int vertex_id, int fire_ray_surface_id, int direction_id, int magnitude_id, int time_id, int radius_id); // adds new load to loads_data
  bool modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2); // modify a load
  bool delete_load(int load_id); // deletes load from loads_data
  bool add_time(std::string time_id, std::string t_begin, std::string t_end); // adds new time to time_data
  bool add_radius(std::string radius_id, std::string radius); // adds new radius to radius_data
  bool add_direction(std::string direction_id, std::string x, std::string y, std::string z); // adds new direction to direction_data
  bool add_magnitude(std::string magnitude_id, std::string magnitude_value); // adds new magnitude to magnitude_data
  bool add_fire_ray_surface(int fire_ray_surface_id, int surface_id); // adds new surface id to fire_ray_surface_data
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  int  get_time_data_id_from_time_id(int time_id); // searches for the time_id in the time_data and returns the indices or -1 if it fails
  int  get_radius_data_id_from_radius_id(int radius_id); // searches for the radius_id in the radius_data and returns the indices or -1 if it fails
  int  get_magnitude_data_id_from_magnitude_id(int magnitude_id); // searches for the magnitude_id in the magnitude_data and returns the indices or -1 if it fails
  int  get_direction_data_id_from_direction_id(int direction_id); // searches for the time_delay_id in the direction_data and returns the indices or -1 if it fails
  std::vector<int> get_fire_ray_surface_data_ids_from_fire_ray_surface_id(int fire_ray_surface_id); // searches for the fire_ray_surface_id in the fire_ray_surface_data and returns an empty vector if it fails
  std::vector<int> get_fire_ray_surface_ids_from_fire_ray_surface_id(int fire_ray_surface_id); //returns surface ids for fire ray shoot
  
  std::vector<int> get_node_ids(int load_id); //returns node ids for curve
  std::vector<int> get_edge_ids(int load_id); //returns edge ids for curve
  std::vector<std::vector<double>> get_hit_coordinates(int load_id); //returns hit coordinates
  std::vector<std::vector<int>> get_face_ids(int load_id); //returns face ids for the node ids from get_node_ids
  std::vector<std::vector<double>> get_times(int load_id); //returns time begin and end ordered by the node ids from get_node_ids
  bool prepare_export(); // prepare sidesets, amplitudes, dflux -> will be prepared for ALL trajectories
  bool clean_export(); // delete prepared
  std::string get_load_export(int load_id); // gets the optional parameters for the export in the steps
  std::string print_data(); // prints out the loads_data

  template <typename T>  std::vector<std::size_t> sort_permutation(const std::vector<T>& vec);
  template <typename T> void apply_permutation(std::vector<T>& vec,const std::vector<std::size_t>& p);
 
  CalculiXCoreInterface *ccx_iface;
};

#endif // CORELOADSTRAJECTORY_HPP