#ifndef CORERESULTSFRD_HPP
#define CORERESULTSFRD_HPP

class CalculiXCoreInterface;

class CoreResultsFrd
{

public:
  CoreResultsFrd();
  ~CoreResultsFrd();

  //header data
  std::vector<std::vector<std::string>> header;
  
  //materials
  std::vector<std::vector<std::string>> materials;
  //materials[0][0] material id
  //materials[0][1] material name

  //nodal data
  std::vector<std::vector<int>> nodes;
  // node[0][0] node id
  // node[0][1] node_coord data_id
  std::vector<std::vector<double>> nodes_coords;
  //node_coord[0][0] x coordinate
  //node_coord[0][1] y coordinate
  //node_coord[0][2] z coordinate

  //element data
  std::vector<std::vector<int>> elements;
  // elements[0][0] element id
  // elements[0][1] element type
  // elements[0][2] element connectivity data id
  // elements[0][3] material id
  std::vector<std::vector<int>> elements_connectivity;
  //elements_connectivity[0][0] first node and so on

  int current_step_id = -1;
  int current_step_time_id = -1;
  std::vector<std::vector<int>> steps;
  // steps[0][0] step id
  // steps[0][1] step time data id
  // steps[0][2] result type
  // 1 DISP
  // 2 STRESS
  // 3 TOSTRAIN
  // 4 ERROR
  // steps[0][3] result data id
  std::vector<double> steps_time;
  // steps_time[0] step time
  std::vector<std::vector<std::vector<double>>> result_disp;
  //result_disp[step][node][component]
  //result_disp[0][0][0] D1
  //result_disp[0][0][1] D2
  //result_disp[0][0][2] D3
  //result_disp[0][0][3] ALL
  std::vector<std::vector<std::vector<int>>> result_disp_nodes;
  //result_disp_nodes[step][node][connection]
  //result_disp_nodes[0][0][0] node_id
  //result_disp_nodes[0][0][1] result_disp_node_data_id
  
  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;
  int current_read_mode = 0; //0 header 1 nodal data 2 element data 3 parameter data

  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool clear(); // clear all stored result data
  bool check_initialized(); // check if object is initialized
  bool read(); // reads the frd file
  std::vector<std::string> split_line(std::string frdline); // splits the line to a vector
  bool check_mode(std::vector<std::string> line); // checks if the current read mode is still valid or others should be used
  bool read_header(std::vector<std::string> line); // processing the header data
  bool read_node(std::vector<std::string> line); // processing the nodes data
  bool read_element(std::vector<std::string> line); // processing the elements data
  bool read_parameter_header(std::vector<std::string> line); // processing the parameter header
  bool check_current_step_id_exists(int step_id); // checks if the current step is already created
  bool print_data(); // prints the data to the console

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSFRD_HPP