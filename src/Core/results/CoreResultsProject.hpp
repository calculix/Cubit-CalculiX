#ifndef CORERESULTSPROJECT_HPP
#define CORERESULTSPROJECT_HPP

#include <vector>
#include <string>
#include <chrono>
class CoreResultsFrd;
class ProgressTool;
class MeshExportInterface;

class CoreResultsProject
{

public:
  CoreResultsProject();
  ~CoreResultsProject();

  bool is_initialized = false;

  bool init(CoreResultsFrd* frd); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool clear(); // clear all stored result data
  bool project(int step, int totalincrement,double scale); // project displacements
  bool project_topology(int topology_id,int iteration_id = -1,double threshold = 0.5, bool preview = false);
  bool checkLinkPossible(); // checks if the linking of results to the current cubit model is possible
  int get_result_blocks_data_id_step(int step); // gets the result blocks data id for the desired step, -1 if fails
  int get_result_blocks_data_id_totalincrement(int totalincrement); // gets the result blocks data id for the desired totalincrement, -1 if fails

  ProgressTool *progressbar; // progressbar
  CoreResultsFrd *frd; // the current frd file that will be written
  MeshExportInterface *me_iface;
};

#endif // CORERESULTSPROJECT_HPP