#ifndef CORERESULTSDAT_HPP
#define CORERESULTSDAT_HPP

#include <vector>
#include <string>
#include <thread>
#include "ProgressTool.hpp"

class CalculiXCoreInterface;

class CoreResultsDat
{

public:
  CoreResultsDat();
  ~CoreResultsDat();

  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;
  int current_read_mode = -1; //1 result header 2 result line

  int current_result_block = -1;
  int current_total_time_id = -1;
  int current_step = -1;
  int current_increment = -1;
  
  std::vector<int> progress; // size max_threads + total
  std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();

  std::vector<std::vector<int>> result_blocks;
  // result_blocks[0][0] result block
  // result_blocks[0][1] total time data id
  // result_blocks[0][2] result block type
  // result_blocks[0][3] result block set
  // result_blocks[0][4] result block data id
  // result_blocks[0][5] step
  // result_blocks[0][6] step increment

  std::vector<double> total_times;
  // total_times[0] total time

  std::vector<std::vector<std::string>> result_block_components;
  // result_block_components[0][0] component 1
  // result_block_components[0][1] component 2
  // result_block_components[0][2] .....

  std::vector<std::string> result_block_type;
  // result_block_type[0] displacements
  // result_block_type[1] stresses
  // result_block_type[2] forces 
  // ...

  std::vector<std::string> result_block_set;
  // result_block_set[0] nodeset_1
  // result_block_set[1] block_sample
  // ...

  std::vector<std::vector<std::vector<double>>> result_block_data;
  //result_block_data[result block data id][node/element][component]
  //result_block_data[0][0][0] component...

  std::vector<std::vector<std::vector<int>>> result_block_c1_data; //column 1 = c1
  //result_block_c1_data[step][node/element][connection]
  //result_block_c1_data[0][0][0] c1 
  //result_block_c1_data[0][0][1] result_block_c1_data_id
  //result_block_c1_data[0][0][2] type 1 node 2 element

  std::vector<std::vector<std::vector<double>>> buckle_data; // contains all buckling data from dat file
  //buckle_data[set][line][data]
  //buckle_data[0][0][0] mode no
  //buckle_data[0][0][1] buckling factor

  std::vector<std::vector<int>> result_section;
  //result_section[0][0] result set
  //result_section[0][1] data_id

  std::vector<std::string> result_section_set;
  //result_section_set[0] result set
  
  std::vector<std::vector<double>> result_section_data;
  //result_section_data[data_id][0] time
  //result_section_data[data_id][1] increment
  //result_section_data[data_id][2] total surface force fx
  //result_section_data[data_id][3] total surface force fy
  //result_section_data[data_id][4] total surface force fz
  //result_section_data[data_id][5] moment about the origin mx 
  //result_section_data[data_id][6] moment about the origin my
  //result_section_data[data_id][7] moment about the origin mz
  //result_section_data[data_id][8] center of gravity 1
  //result_section_data[data_id][9] center of gravity 2
  //result_section_data[data_id][10] center of gravity 3
  //result_section_data[data_id][11] mean normal 1
  //result_section_data[data_id][12] mean normal 2
  //result_section_data[data_id][13] mean normal 3
  //result_section_data[data_id][14] moment about the center of gravity mx
  //result_section_data[data_id][15] moment about the center of gravity my
  //result_section_data[data_id][16] moment about the center of gravity mz
  //result_section_data[data_id][17] area
  //result_section_data[data_id][18] normal force
  //result_section_data[data_id][19] shear force
  //result_section_data[data_id][20] torque moment
  //result_section_data[data_id][21] bending moment

  std::vector<std::string> result_section_label; //contains the labels for the data
  
  std::vector<std::vector<std::vector<std::string>>> dat_arrays; //contains the result block lines

  // sorted vectors for faster results search
  // will probably be used for python interface queries
  std::vector<std::vector<int>> sorted_c1;
  std::vector<std::vector<int>> sorted_result_block_c1_data_id;
  std::vector<std::vector<int>> sorted_result_block_c1_data_type;

  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool clear(); // clear all stored result data
  bool read(); // reads the dat file
  bool read_single(); // reads the dat file
  bool read_parallel(); // reads the dat file
  std::vector<std::string> split_line(std::string line); // splits the line to a vector
  bool is_number(std::string number); // check if string is a positive number, for check of first column
  bool is_whitespace(std::string line); // check if line consists only of whitespace
  std::string erase_whitespace(std::string line); // erases all whitespaces
  bool check_mode(std::vector<std::string> line); // checks if the current read mode is still valid or others should be used
  bool check_step(std::vector<std::string> line); // checks if the current step or increment should be updated
  bool read_header(std::vector<std::string> line); // processing the result block header
  bool header_emas(std::vector<std::string> line); // processing emas header
  bool header_section_1(std::vector<std::string> line); // processing section header 1
  bool header_section_2(std::vector<std::string> line); // processing section header 2
  bool header_section_3(std::vector<std::string> line); // processing section header 3
  bool header_section_4(std::vector<std::string> line); // processing section header 4
  bool header_section_5(std::vector<std::string> line); // processing section header 5
  bool read_line(std::vector<std::string> line); // processing the result lines
  bool read_line_buckle(std::vector<std::string> line); // processing the result lines for buckling output
  bool read_line_section_1(std::vector<std::string> line); // processing the result lines for section output
  bool read_line_section_2(std::vector<std::string> line); // processing the result lines for section output
  bool read_line_section_3(std::vector<std::string> line); // processing the result lines for section output
  bool read_line_section_4(std::vector<std::string> line); // processing the result lines for section output
  bool read_lines_thread(int result_block_data_id, int thread_id); // processing the result lines for result block
  bool compute_predefined(int result_block_data_id, int thread_id); // computing the predefined calculations
  int get_current_result_block_type(std::string result_type); // gets result_block_type
  int get_current_result_block_set(std::string result_set); // gets result_block_set
  int get_current_result_section_set(std::string result_set); // gets result_section_set
  bool check_element_sets(); // checks if the data for the element sets has integration points data or not...if yes set prefix ip_
  std::vector<std::string> get_result_block_components_from_result_block_type(std::string result_block_type); // get components from block type
  int get_result_block_component_id(int result_block_type_id,std::string result_block_component); // get component id
  int get_result_block_type_data_id(std::string result_block_type); // returns the result_block_type_data_id or returns -1 
  int get_result_block_set_data_id(std::string result_block_set); // returns the result_block_set_data_id or returns -1 
  bool sort_data(int data_id); // sorts the data
  int get_result_block_lines(std::vector<std::vector<std::vector<std::string>>> result_block_lines); // computes and returns the maximum line numbers that will be read for the result blocks
  int get_result_blocks_predefined(); // gets the number of result blocks that are needing a predefined calculation
  void update_progressbar(); // updates the percent from the progressbar
  bool print_data(); // prints the data to the console

  template <typename T>  std::vector<std::size_t> sort_permutation(const std::vector<T>& vec);
  template <typename T> void apply_permutation(std::vector<T>& vec,const std::vector<std::size_t>& p);

  ProgressTool *progressbar; // progressbar
  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSDAT_HPP