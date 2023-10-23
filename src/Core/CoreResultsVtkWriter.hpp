#ifndef CORERESULTSVTKWRITER_HPP
#define CORERESULTSVTKWRITER_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;
class CoreResultsFrd;
class CoreResultsDat;
class ProgressTool;
class MeshExportInterface;

class CoreResultsVtkWriter
{

public:
  CoreResultsVtkWriter();
  ~CoreResultsVtkWriter();
    
  int job_id = -1;
  int write_mode = -1; //1 linked //2 unlinked
  std::string filepath = "";
  bool is_initialized = false;
  int current_offset = 0;
  int max_increments = 0;
  int current_increment = 0;
  double current_time = 0;
  int maxDataRows = 0;
  int currentDataRow = 0;
  double rangeMin = 0;
  double rangeMax = 0;
  int nparts = 0;
  int nparts_dat = 0; // number of parts from dat file
  int current_part = 0;
  std::string current_filepath_vtu = "";
  std::string current_filepath_vtpc = "";
  std::vector<std::string> filepath_vtu; // to store the filepaths
  std::vector<std::string> part_names; // to store the parts name
  std::vector<int> part_ids; // to store the parts ids
  std::vector<int> block_ids;
  std::vector<std::vector<int>> block_node_ids;
  std::vector<std::vector<int>> block_element_ids;
  std::vector<int> nodeset_ids;
  std::vector<std::vector<int>> nodeset_node_ids;
  std::vector<int> sideset_ids;
  std::vector<std::vector<int>> sideset_node_ids;
  std::vector<std::vector<int>> sideset_elements;
  //sideset_elements[sideset id][element][0] element id
  std::vector<std::vector<int>> sideset_elements_type;
  std::vector<std::vector<std::vector<int>>> sideset_elements_connectivity;
  //sideset_elements_connectivity[sideset id][element][0] node 1
  std::vector<std::vector<int>> element_id_type_connectivity; // to store the element data

  bool init(int job_id,CoreResultsFrd* frd,CoreResultsDat* dat); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool clear(); // clear all stored result data
  bool clearLinked(); // clear all stored result data
  bool check_initialized(); // check if object is initialized
  bool write(); // write to paraview file formats
  bool write_linked(); // write part to paraview file formats if linking is possible
  bool write_vtpc(); // write to paraview file formats
  bool write_vtu_linked(); // write only paraview vtu file format, blocks ect could not be linked
  bool write_vtu_unlinked(); // write only paraview vtu file format, blocks ect could not be linked
  bool write_to_file(std::string filepath, std::string &content); // write the content to file
  int getMaxDataRows(); // get # of data rows from frd and dat
  bool checkResults(); // get # of increments from frd and dat
  bool checkLinkPossible(); // checks if the linking of results to the current cubit model is possible
  bool checkResultsLinked(); // get # of increments from frd and dat
  bool checkDatTimeValueExists(double total_time); // checks if the dat totaltime exists
  std::string level_whitespace(int level); // gets the whitespace for the level in the xml file
  bool rewrite_connectivity_unlinked(); // rewrites the nodenumbers in the connectivity for unlinked mode
  std::string get_element_connectivity_vtk(int element_connectivity_data_id, int element_type); // gets the connectivity already converted to vtk format
  std::string get_element_connectivity_vtk_linked(int element_connectivity_data_id, int element_type); // gets the connectivity already converted to vtk format
  std::string get_element_type_vtk(int element_type); // gets the element type already converted to vtk format
  std::string get_element_offset_vtk(int element_connectivity_data_id); // gets the element offset already converted to vtk format
  int getParaviewNode(int frd_node_id); // gets the paraview node id for frd node id
  std::string get_increment(); // gets the current increment in the format 00x
  std::string get_increment_time(); // gets the current increment time value
  int get_step_increment(double total_time); // gets the step increment from a time value from .dat
  int get_max_step_increment(); // gets the max step increment from .dat
  std::vector<int> get_currentincrement_result_blocks_data(); // gets the result blocks data from frd to fuse with dat data
  std::vector<int> get_result_blocks_data_ids(); // gets the result blocks data ids for the current increment
  std::vector<int> get_result_blocks_data_ids_linked(); // gets the result blocks data ids for the current increment from frd_all
  std::vector<int> get_dat_result_blocks_data_ids_linked(int set_id); // gets the result blocks data ids for the current increment from dat_all
  std::vector<int> get_result_block_node_data_id(int result_blocks_data_id); // gets the result blocks node data ids for the current block
  std::vector<int> get_result_block_node_data_id_linked(int result_blocks_data_id); // gets the result blocks node data ids for the current block and set from frd_all
  std::string get_result_data(int data_id, int node_data_id); // gets the result data for a node
  bool link_nodes(); // links the ids from frd/dat all
  bool link_elements(); // links the ids from frd/dat all
  bool link_dat(); // links the dat
  bool prepare_sidesets(); // gets all data for the sidesets
  std::vector<double> get_integration_point_coordinates(int element_type, int ip, int ipmax, std::vector<std::vector<double>> nodes_coords); // computes the integration point coordinates for the given element and ip point number
  std::vector<std::vector<double>> compute_integration_points_displacements(int set_id);

  ProgressTool *progressbar; // progressbar
  CalculiXCoreInterface *ccx_iface;
  CoreResultsFrd *frd; // the current frd file that will be written
  CoreResultsDat *dat; // the current dat file that will be written
  std::vector<CoreResultsFrd*> vec_frd; // the storage of frd files that will be written
  CoreResultsFrd *frd_all; // the input frd file that will be written
  CoreResultsDat *dat_all; // the input dat file that will be written
  MeshExportInterface *me_iface;
};

#endif // CORERESULTSVTKWRITER_HPP