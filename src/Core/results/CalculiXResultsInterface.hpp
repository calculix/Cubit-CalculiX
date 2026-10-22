#ifndef CALCULIXRESULTSINTERFACE_HPP
#define CALCULIXRESULTSINTERFACE_HPP

#include <string>
#include <vector>

class QString;

class CalculiXResultsInterface
{
public:
  bool result_cgx_job(int job_id); // opens the results with cgx
  bool result_paraview_job(int job_id); // opens the results with paraview
  bool result_plot_job_frd(int job_id,int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment,bool x_time,int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath); // plots the results
  bool result_plot_job_dat(int job_id,int x_node_id,int x_element_id,int x_element_ip, std::string x_block_set,std::string x_block_type, std::string x_block_component, bool x_time,int y_node_id,int y_element_id,int y_element_ip, std::string y_block_set,std::string y_block_type, std::string y_block_component, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath); // plots the results
  bool result_plot_job_dat_section(int job_id, std::string x_section_set, std::string x_section_component, bool x_time, std::string y_section_set, std::string y_section_component, bool y_time, QString title, QString x_axis, QString y_axis, bool save, QString save_filepath); // plots the results
  bool result_csv_job_frd(int job_id,std::string block_type, std::string block_component, std::string increment,int node_id,int block_id,int nodeset_id,int sideset_id, bool overwrite, std::string save_filepath); // save the results to csv
  bool result_csv_job_dat(int job_id,std::string block_type,std::string block_set, std::string block_component, std::string time,int node_id,int element_id,bool overwrite, std::string save_filepath); // save the results to csv
  bool result_csv_job_dat_section(int job_id,std::string result_set, std::string time,bool overwrite, std::string save_filepath); // save the results to csv
  bool create_result(int job_id); // adds a new result for job
  bool delete_result(int job_id); // delete result for job
  bool load_result(int job_id); // load result for job
  int convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids); // convert result for job
  bool project_result(int job_id,int step,int totalincrement,double scale); // project disp result for job for specific step or increment
  bool project_topology(int topology_id, int iteration_id = -1,double threshold = 0.5, bool preview = false);
  double compute_von_mises_stress(std::vector<double> vec); // computes the von mises stress/strain for a vector of values
  double compute_von_mises_strain(std::vector<double> vec); // computes the von mises stress/strain for a vector of values
  std::vector<double> compute_principal_stresses(std::vector<double> vec); // computes the principal stress for a vector/matrix of stresses
  std::vector<double> compute_principal_strains(std::vector<double> vec); // computes the principal stress for a vector/matrix of strains
};

#endif // CALCULIXRESULTSINTERFACE_HPP
