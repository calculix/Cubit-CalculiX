#ifndef CALCULIXLOADSINTERFACE_HPP
#define CALCULIXLOADSINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXLoadsInterface
{
public:
  std::vector<int> get_loadsforces_ids(); // get all load forces ids
  std::vector<int> get_loadspressures_ids(); // get all load pressure ids
  std::vector<int> get_loadsheatfluxes_ids(); // get all load heatflux ids
  std::vector<int> get_loadsgravity_ids(); // get all load gravity ids
  std::vector<int> get_loadscentrifugal_ids(); // get all load centrifugal ids
  std::vector<int> get_loadstrajectory_ids(); // get all load trajectory ids
  std::vector<int> get_loadsfilm_ids(); // get all load film ids
  std::vector<int> get_loadsradiation_ids(); // get all load radiation ids
  std::vector<int> get_loadssurfacetraction_ids(); // get all load surface traction ids
  std::vector<int> get_loadsbodyheatflux_ids(); // get all load bodyheatflux ids
  bool delete_loadsforces(int force_id); // delete force load
  bool create_loadsforces(std::vector<std::string> options, std::vector<double> options2); // adds a new force
  bool modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker); // modify a force
  bool delete_loadspressures(int pressure_id);
  bool create_loadspressures(std::vector<std::string> options, std::vector<double> options2); // adds a new pressure
  bool modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker); // modify a pressure
  bool create_loadsheatfluxes(std::vector<std::string> options, std::vector<double> options2);
  bool modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_loadsheatfluxes(int heatflux_id);
  bool create_loadsgravity(std::vector<std::string> options); // adds a new gravity load
  bool modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a gravity
  bool delete_loadsgravity(int gravity_id); // delete gravity load
  bool create_loadscentrifugal(std::vector<std::string> options); // adds a new centrifugal load
  bool modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a centrifugal
  bool delete_loadscentrifugal(int centrifugal_id); // delete centrifugal load
  bool create_loadstrajectory(std::string load_type,std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3); // adds a new trajectory load
  bool modify_loadstrajectory(int trajectory_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3); // modify a trajectory
  bool delete_loadstrajectory(int trajectory_id); // delete trajectory load
  std::string loadstrajectory_get_load_type(int trajectory_id); //returns the load type
  int loadstrajectory_get_subload_id(int trajectory_id); //returns the subload id, -1 if fails
  std::vector<int> loadstrajectory_heatflux_get_node_ids(int trajectory_id); //returns node ids for curve
  std::vector<int> loadstrajectory_heatflux_get_edge_ids(int trajectory_id); //returns edge ids for curve
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_hit_coordinates(int trajectory_id); //returns hit coordinates
  std::vector<std::vector<std::vector<int>>> loadstrajectory_heatflux_get_face_ids(int trajectory_id); //returns face ids for the node ids from get_node_ids
  std::vector<std::vector<std::vector<int>>> loadstrajectory_heatflux_get_draw_face_ids(int trajectory_id); //returns face ids for the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_times(int trajectory_id); //returns time begin and end ordered by the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_radius(int trajectory_id); //returns radius ordered by the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_magnitude(int trajectory_id); //returns magnitude ordered by the node ids from get_node_ids
  std::vector<int> loadstrajectory_bodyheatfluxsphere_get_node_ids(int trajectory_id); //returns node ids for curve
  std::vector<int> loadstrajectory_bodyheatfluxsphere_get_edge_ids(int trajectory_id); //returns edge ids for curve
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(int trajectory_id); //returns hit coordinates
  std::vector<std::vector<std::vector<int>>> loadstrajectory_bodyheatfluxsphere_get_element_ids(int trajectory_id); //returns element ids for the node ids from get_node_ids
  std::vector<std::vector<std::vector<int>>> loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(int trajectory_id); //returns element ids for the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_times(int trajectory_id); //returns time begin and end ordered by the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_radius(int trajectory_id); //returns radius ordered by the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_depth(int trajectory_id); //returns depth ordered by the node ids from get_node_ids
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_magnitude(int trajectory_id); //returns magnitude ordered by the node ids from get_node_ids
  bool create_loadsfilm(std::vector<std::string> options); // adds a new film load
  bool modify_loadsfilm(int film_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a film
  bool delete_loadsfilm(int film_id); // delete film load
  bool create_loadsradiation(std::vector<std::string> options); // adds a new radiation load
  bool modify_loadsradiation(int radiation_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a radiation
  bool delete_loadsradiation(int radiation_id); // delete radiation load
  bool create_loadssurfacetraction(std::vector<std::string> options, std::vector<double> options2); // adds a new surfacetraction load
  bool modify_loadssurfacetraction(int surfacetraction_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker); // modify a surface traction
  bool delete_loadssurfacetraction(int surfacetraction_id); // delete surfacetraction load
  bool create_loadsbodyheatflux(std::vector<std::string> options, std::vector<int> options2); // adds a new bodyheatflux load
  bool modify_loadsbodyheatflux(int bodyheatflux_id, std::vector<std::string> options, std::vector<int> options2, std::vector<int> options_marker); // modify a bodyheatflux
  bool delete_loadsbodyheatflux(int bodyheatflux_id); // delete bodyheatflux load
  std::vector<std::vector<std::string>> get_loadsforces_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadspressures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsheatfluxes_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsgravity_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadscentrifugal_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadstrajectory_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsfilm_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsradiation_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadssurfacetraction_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsbodyheatflux_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXLOADSINTERFACE_HPP
