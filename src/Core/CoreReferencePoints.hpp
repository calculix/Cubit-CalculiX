#ifndef COREREFERENCEPOINTS_HPP
#define COREREFERENCEPOINTS_HPP

class CalculiXCoreInterface;

class CoreReferencePoints
{

public:
  CoreReferencePoints();
  ~CoreReferencePoints();

  // needed to handle the reference points on export
  
  std::vector<std::vector<int>> referencepoints_data; // used to store the connection between a vertex id and its nodes
  // referencepoints_data[0][0] vertex_id
  // referencepoints_data[0][1] ref_node
  // referencepoints_data[0][2] rot_node

  int rot_max_node_id;

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the constraints
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool update_on_export(int max_node_id); // fills the referencepoints_data with the vertices and nodes
  bool create_referencepoint(int vertex_id); // adds new reference point
  bool delete_referencepoint(int vertex_id); // deletes reference point from referencepoints_data
  int  get_referencepoint_data_id_from_vertex_id(int vertex_id); // searches for the vertex_id in the referencepoints_data and returns the indices or -1 if it fails
  int  get_node_id_from_vertex_id(int vertex_id); // get node_id for the vertex_id  and returns -1 if it fails
  std::array<double,3> get_coords_from_node_id(int node_id); // get the coords from node
  int  get_ref_from_vertex_id(int vertex_id); // get ref_node for the vertex_id  and returns -1 if it fails
  int  get_rot_from_vertex_id(int vertex_id); // get ref_node for the vertex_id  and returns -1 if it fails
  
  std::string get_referencepoints_export(); // get CalculiX referencepoints exports
  std::string print_data(); // prints out the referencepoints_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREREFERENCEPOINTS_HPP
