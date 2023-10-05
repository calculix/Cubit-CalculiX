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
  std::vector<std::vector<int>> elements_connectivity;
  //elements_connectivity[0][0] first node and so on

  //element data

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
  bool print_data(); // prints the data to the console

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSFRD_HPP