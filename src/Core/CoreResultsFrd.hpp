#ifndef CORERESULTSFRD_HPP
#define CORERESULTSFRD_HPP

#include "ProgressTool.hpp"
#include <vector>
#include <string>
#include <thread>
#include <chrono>

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

  int current_result_block = -1;
  int current_step = -1;
  int current_step_increment = -1;
  int current_total_increment = -1;
  int current_total_time_id = -1;
  
  std::vector<std::vector<int>> result_blocks;
  // result_blocks[0][0] result block
  // result_blocks[0][1] step
  // result_blocks[0][2] step increment
  // result_blocks[0][3] total increment
  // result_blocks[0][4] total time data id
  // result_blocks[0][5] result block type
  // result_blocks[0][6] result block data id

  std::vector<double> total_times;
  // total_time[0] total time

  std::vector<std::vector<std::string>> result_block_components;
  // result_block_components[0][0] component 1
  // result_block_components[0][1] component 2
  // result_block_components[0][2] .....

  std::vector<std::string> result_block_type;
  // result_block_type[0] disp
  // result_block_type[1] stress
  // result_block_type[2] forc 
  // ...

  std::vector<std::vector<std::vector<double>>> result_block_data;
  //result_block_data[result block data id][node][component]
  //result_block_data[0][0][0] component...

  std::vector<std::vector<std::vector<int>>> result_block_node_data;
  //result_block_node_data[step][node][connection]
  //result_block_node_data[0][0][0] node_id
  //result_block_node_data[0][0][1] result_block_node_data_id
  
  // sorted vectors for faster results search
  // will probably be used for python interface queries and vtk writer linking
  std::vector<int> sorted_node_ids; //frd nodes
  std::vector<int> sorted_node_data_ids; //frd nodes
  std::vector<std::vector<int>> sorted_result_node_ids; //result blocks
  std::vector<std::vector<int>> sorted_result_node_data_ids; //result blocks
  
  
  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;
  int current_read_mode = 0; //0 header 1 nodal data 2 element data 3 parameter data 4 result block
  std::vector<std::string> keys; // keys for all lines
  std::vector<std::vector<std::vector<std::string>>> frd_arrays; //contains the result block lines
  std::vector<int> progress; // size max_threads + total
  std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
  
  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool clear(); // clear all stored result data
  bool check_initialized(); // check if object is initialized
  bool read(); // reads the frd file
  bool read_single(); // reads the frd file single core
  bool read_parallel(); // reads the frd file in parallel for the result blocks
  std::vector<std::string> split_line(std::string frdline); // splits the line to a vector
  std::string get_key(std::string frdline); // gets the frd key, first 6 chars in the line without whitespace
  bool check_mode(std::vector<std::string> line); // checks if the current read mode is still valid or others should be used
  bool check_mode_by_key(std::string key); // checks if the current read mode is still valid or others should be used
  bool read_header(std::vector<std::string> line); // processing the header data
  bool read_node(std::vector<std::string> line); // processing the nodes data
  bool read_nodes_thread(int start,int end, int data_start, int thread_id); // processing the nodes data
  bool read_element(std::vector<std::string> line); // processing the elements data
  bool read_elements_thread(int start,int end, int data_start,int thread_id); // processing the nodes data
  bool read_parameter_header(std::vector<std::string> line); // processing the parameter header
  int get_current_result_block_type(std::string result_type); // gets result_block_type
  bool read_nodal_result_block(std::vector<std::string> line); // processing the nodal result block
  bool read_nodal_result_block_add_components(std::vector<std::string> line); // adds components if needed to the nodal result block
  bool read_nodal_result_block_thread(int result_block_data_id,int thread_id); // processing the nodal result block in the threads
  std::vector<std::string> get_result_block_components_from_result_block_type(std::string result_block_type); // get components from block
  int get_result_block_component_id(int result_block_type_id,std::string result_block_component); // get component id
  std::vector<int> get_node_range(); // scans the frd for the node range
  std::vector<int> get_element_range(); // scans the frd for the element range and total number of elements
  void set_element_range_data_start(std::vector<std::vector<int>> &thread_ranges); // scans the ranges and counts the elements in the ranges to set the data_start
  bool insert_key_thread(int start, int end,int thread_id); // inserts the keys for range
  bool check_key(std::string key, int line); // checks if the key is at the given line
  bool sort_result_block_node_data(int data_id); // sorts the result block node data
  int get_result_block_lines(); // computes and returns the maximum line numbers that will be read for the result blocks
  void update_progressbar(); // updates the percent from the progressbar
  bool print_data(); // prints the data to the console

  template <typename T>  std::vector<std::size_t> sort_permutation(const std::vector<T>& vec);
  template <typename T> void apply_permutation(std::vector<T>& vec,const std::vector<std::size_t>& p);

  ProgressTool *progressbar; // progressbar
  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSFRD_HPP