#ifndef CORERESULTSVTKWRITER_HPP
#define CORERESULTSVTKWRITER_HPP

class CalculiXCoreInterface;
class CoreResultsFrd;
class CoreResultsDat;

class CoreResultsVtkWriter
{

public:
  CoreResultsVtkWriter();
  ~CoreResultsVtkWriter();
  
  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;
  int current_offset = 0;
  int max_increments = 0;
  int current_increment = 0;
  int maxDataRows = 0;
  int currentDataRow = 0;
  double rangeMin = 0;
  double rangeMax = 0;

  bool init(int job_id,CoreResultsFrd* frd,CoreResultsDat* dat); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool clear(); // clear all stored result data
  bool check_initialized(); // check if object is initialized
  bool write(); // write to paraview file formats
  bool write_to_file(std::string filepath, std::string &content); // write the content to file
  int getMaxDataRows(); // get # of data rows from frd and dat
  bool checkResults(); // get # of increments from frd and dat
  std::string level_whitespace(int level); // gets the whitespace for the level in the xml file
  std::string get_element_connectivity_vtk(int element_connectivity_data_id, int element_type); // gets the connectivity already converted to vtk format
  std::string get_element_type_vtk(int element_type); // gets the element type already converted to vtk format
  std::string get_element_offset_vtk(int element_connectivity_data_id); // gets the element offset already converted to vtk format
  std::vector<int> get_result_blocks_data_ids(); // gets the result blocks data ids for the current increment
  std::vector<int> get_result_block_node_data_id(int result_blocks_data_id); // gets the result blocks node data ids for the current block
  std::string get_result_data(int data_id, int node_data_id); // gets the result data for a node

  CalculiXCoreInterface *ccx_iface;
  CoreResultsFrd *frd;
  CoreResultsDat *dat;
};

#endif // CORERESULTSVTKWRITER_HPP