#ifndef CORECONSTRAINTS_HPP
#define CORECONSTRAINTS_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreConstraints
{

public:
  CoreConstraints();
  ~CoreConstraints();

  // make 1 data storage for each constraint type
  // rigid body
  // tie
  // equation
  // equation group

  std::vector<std::vector<int>> constraints_data; // used to store the connection between a constraint id and constraint type id
  // constraints_data[0][0] constraint_id
  // constraints_data[0][1] constraint_type
  // constraints_data[0][2] constraint_type_id

  std::vector<std::vector<std::string>> rigidbody_constraint_data; // type 1
  // rigidbody_constraint_data[0][0] rigidbody_constraint_id
  // rigidbody_constraint_data[0][1] type 1:nodeset 2:block
  // rigidbody_constraint_data[0][2] type id
  // rigidbody_constraint_data[0][3] vertex REF
  // rigidbody_constraint_data[0][4] vertex ROT
  // rigidbody_constraint_data[0][5] option: 
  
  std::vector<std::vector<std::string>> tie_constraint_data; // type 2
  // tie_constraint_data[0][0] tie_constraint_id
  // tie_constraint_data[0][1] name
  // tie_constraint_data[0][2] master sideset
  // tie_constraint_data[0][3] slave sideset
  // tie_constraint_data[0][4] option: position_tolerance

  std::vector<std::vector<std::string>> equation_constraint_data; // type 3
  // equation_constraint_data[0][0] equation_constraint_id
  // equation_constraint_data[0][1] name
  
  std::vector<std::vector<double>> equation_data;
  // equation_data[0][0] equation_constraint_id
  // equation_data[0][1] node_id
  // equation_data[0][2] dof
  // equation_data[0][3] coefficient
  
  std::vector<std::vector<std::string>> equation_group_constraint_data; // type 4
  // equation_group_constraint_data[0][0] equation_constraint_id
  // equation_group_constraint_data[0][1] name
  
  std::vector<std::vector<double>> equation_group_data;
  // equation_group_data[0][0] equation_constraint_id
  // equation_group_data[0][1] node_id_1
  // equation_group_data[0][2] node_id_2
  // equation_group_data[0][3] dof
  

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the constraints
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_constraint(std::string constraint_type, std::vector<std::string> options, std::vector<std::vector<double>> options2); // adds new constraint
  bool modify_constraint(std::string constraint_type, int constraint_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<double>> options2); // modify a constraint
  bool delete_constraint(int constraint_id); // deletes constraint from constraints_data
  bool add_constraint(int constraint_id, int constraint_type, int constraint_type_id); // adds new constraint to constraints_data
  bool add_rigidbody_constraint(std::string rigid_body_constraint_id, std::string entity_type, std::string type_id,std::string vertex_ref,std::string vertex_rot); // adds new rigid_body to rigidbody_constraint_data
  bool add_tie_constraint(std::string tie_constraint_id, std::string name, std::string master,std::string slave,std::string position_tolerance); // adds new tie to tie_constraint_data
  bool add_equation_constraint(std::string equation_constraint_id, std::string name); // adds new equation to equation_constraint_data
  bool add_equation(double equation_constraint_id, double node_id, double dof, double coefficient); // adds new equation to equation_data
  bool add_equation_group_constraint(std::string equation_constraint_id, std::string name); // adds new equation to equation_constraint_data
  bool add_equation_group(double equation_constraint_id, double node_id_1, double node_id_2, double dof); // adds new equation to equation_data
  int  get_constraints_data_id_from_constraint_id(int constraint_id); // searches for the constraint_id in the constraints_data and returns the indices or -1 if it fails
  int  get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(int rigidbody_constraint_id); // searches for the rigidbody_constraint_id in the rigidbody_constraint_data and returns the indices or -1 if it fails
  int  get_tie_constraint_data_id_from_tie_constraint_id(int tie_constraint_id); // searches for the tie_constraint_id in the tie_constraint_data and returns the indices or -1 if it fails
  int  get_equation_constraint_data_id_from_equation_constraint_id(int equation_constraint_id); // searches for the equation_constraint_id in the equation_constraint_data and returns the indices or -1 if it fails
  int  get_equation_group_constraint_data_id_from_equation_group_constraint_id(int equation_group_constraint_id); // searches for the equation_constraint_id in the equation_constraint_data and returns the indices or -1 if it fails
  std::vector<int> get_equation_data_ids_from_equation_constraint_id(int equation_constraint_id); // searches for the equation_data_ids in the equation_data and returns a vector with the indices or an empty vector if it fails
  std::vector<int> get_equation_group_data_ids_from_equation_group_constraint_id(int equation_group_constraint_id); // searches for the equation_data_ids in the equation_data and returns a vector with the indices or an empty vector if it fails
  int  get_node_id_from_vertex_id(int vertex_id); // get node_id for the vertex_id  and returns -1 if it fails
  std::string get_constraint_export(); // get CalculiX constraint exports
  std::vector<int> get_rigidbody_vertex_list(); // get list of rigid body vertices
  bool create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3);
  std::string print_data(); // prints out the blocks_data

  CalculiXCoreInterface *ccx_iface;

  template <typename T>  std::vector<std::size_t> sort_permutation(const std::vector<T>& vec);
  template <typename T> void apply_permutation(std::vector<T>& vec,const std::vector<std::size_t>& p);
};

#endif // CORECONSTRAINTS_HPP
