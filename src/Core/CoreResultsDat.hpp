#ifndef CORERESULTSDAT_HPP
#define CORERESULTSDAT_HPP

class CalculiXCoreInterface;

class CoreResultsDat
{

public:
  CoreResultsDat();
  ~CoreResultsDat();

  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;
  int current_read_mode = 0; //0 result block header 1 result block header

  int current_result_block = -1;
  int current_total_increment = -1;
  int current_total_time_id = -1;

  std::vector<std::vector<int>> result_blocks;
  // result_blocks[0][0] result block
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
  //result_block_node_data[step][node/element][connection]
  //result_block_node_data[0][0][0] c1 
  //result_block_node_data[0][0][1] result_block_c1_data_id

  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool clear(); // clear all stored result data
  bool read(); // reads the dat file
  std::vector<std::string> split_line(std::string line); // splits the line to a vector
  bool is_number(std::string number); // check if string is a positive number, for check of first column

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSDAT_HPP