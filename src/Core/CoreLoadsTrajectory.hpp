#ifndef CORELOADSTRAJECTORY_HPP
#define CORELOADSTRAJECTORY_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreLoadsTrajectoryHeatflux;
class CoreLoadsTrajectoryBodyHeatfluxSphere;

class CoreLoadsTrajectory
{

public:
  CoreLoadsTrajectory();
  ~CoreLoadsTrajectory();

  std::vector<std::vector<int>> loads_data; // used to store the connection between a trajectory and its possible options
  // loads_data[0][0] load_id
  // loads_data[0][1] load_type-> 1 heatflux | 2 bodyheatflux
  // loads_data[0][2] sub_load_id

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes 
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_load(std::string load_type,std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3); // adds new load
  bool add_load(int load_id, int load_type, int sub_load_id); // adds new load to loads_data
  bool modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3); // modify a load
  bool delete_load(int load_id); // deletes load from loads_data
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  std::string get_load_type(int load_id); // gets the load type or "" if it fails
  std::string get_name_from_load_id(int load_id); // gets the name from the subloads or "" if it fails
  bool prepare_export(); // prepare exports for the loads
  bool clean_export(); // delete prepared
  std::string print_data(); // prints out the loads_data
 
  CalculiXCoreInterface *ccx_iface;
  CoreLoadsTrajectoryHeatflux *heatflux;
  CoreLoadsTrajectoryBodyHeatfluxSphere *bodyheatfluxsphere;
};

#endif // CORELOADSTRAJECTORY_HPP