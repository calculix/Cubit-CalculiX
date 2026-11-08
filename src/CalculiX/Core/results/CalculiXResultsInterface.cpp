#include "CalculiXResultsInterface.hpp"
#include "CoreResults.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "CubitMessage.hpp"
#include "CoreResultsMath.hpp"
#include "CoreResultsPlot.hpp"
#include "CoreResultsCsv.hpp"
#include <string>
#include <vector>
#include <QString>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXResultsInterface::prepare_vtk_plan(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan)
{
  if (!ccx_core.results)
  {
    PRINT_ERROR("Results core is not initialized.\n");
    return false;
  }
  return ccx_core.results->prepare_vtk_plan(job_id, options, plan);
}

SimResults::ConversionStatus CalculiXResultsInterface::get_vtk_status(int job_id) const
{
  return ccx_core.results ? ccx_core.results->get_vtk_status(job_id) : SimResults::ConversionStatus::NotConverted;
}

bool CalculiXResultsInterface::build_vtk_result(int job_id, const CoreResultsVtkOptions& options)
{
  if (!ccx_core.results)
  {
    PRINT_ERROR("Results core is not initialized.\n");
    return false;
  }
  return ccx_core.results->build_vtk_result(job_id, options);
}

bool CalculiXResultsInterface::cancel_vtk_build()
{
  return ccx_core.results && ccx_core.results->cancel_vtk_build();
}

bool CalculiXResultsInterface::export_vtk_result(int job_id, const std::string& destination)
{
  return ccx_core.results && ccx_core.results->export_vtk_result(job_id, destination);
}

std::string CalculiXResultsInterface::get_vtk_export_path(int job_id) const
{
  return ccx_core.results ? ccx_core.results->get_vtk_export_path(job_id) : std::string{};
}

bool CalculiXResultsInterface::is_vtk_build_active() const
{
  return ccx_core.results && ccx_core.results->is_vtk_build_active();
}

std::shared_ptr<const SimResults::VtkResultsData> CalculiXResultsInterface::get_vtk_result(int job_id) const
{
  return ccx_core.results ? ccx_core.results->get_vtk_result(job_id) : nullptr;
}



bool CalculiXResultsInterface::result_plot_job_frd(int job_id,int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment,bool x_time,int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  return CoreResultsPlot(*ccx_core.results).result_plot_job_frd(job_id,x_node_id,x_block_type,x_block_component,x_increment,x_time,y_node_id,y_block_type,y_block_component,y_increment,y_time,title,x_axis,y_axis,save, save_filepath);
}

bool CalculiXResultsInterface::result_plot_job_dat(int job_id,int x_node_id,int x_element_id,int x_element_ip, std::string x_block_set,std::string x_block_type, std::string x_block_component, bool x_time,int y_node_id,int y_element_id,int y_element_ip, std::string y_block_set,std::string y_block_type, std::string y_block_component, bool y_time, QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  return CoreResultsPlot(*ccx_core.results).result_plot_job_dat(job_id,x_node_id, x_element_id, x_element_ip, x_block_set, x_block_type, x_block_component, x_time,y_node_id,y_element_id,y_element_ip, y_block_set,y_block_type, y_block_component, y_time,title,x_axis,y_axis,save,save_filepath);
}

bool CalculiXResultsInterface::result_plot_job_dat_section(int job_id, std::string x_section_set, std::string x_section_component, bool x_time, std::string y_section_set, std::string y_section_component, bool y_time, QString title, QString x_axis, QString y_axis, bool save, QString save_filepath)
{
  return CoreResultsPlot(*ccx_core.results).result_plot_job_dat_section(job_id, x_section_set, x_section_component, x_time, y_section_set, y_section_component, y_time,title,x_axis,y_axis,save,save_filepath);
}

bool CalculiXResultsInterface::result_csv_job_frd(int job_id,std::string block_type, std::string block_component, std::string increment,int node_id,int block_id,int nodeset_id,int sideset_id, bool overwrite, std::string save_filepath)
{
  return CoreResultsCsv(*ccx_core.results).result_csv_job_frd(job_id, block_type, block_component, increment, node_id, block_id, nodeset_id, sideset_id, overwrite, save_filepath);
}

bool CalculiXResultsInterface::result_csv_job_dat(int job_id,std::string block_type,std::string block_set, std::string block_component, std::string time,int node_id,int element_id,bool overwrite, std::string save_filepath)
{
  return CoreResultsCsv(*ccx_core.results).result_csv_job_dat(job_id, block_type, block_set, block_component, time, node_id, element_id, overwrite, save_filepath);
}

bool CalculiXResultsInterface::result_csv_job_dat_section(int job_id,std::string result_set, std::string time, bool overwrite, std::string save_filepath)
{
  return CoreResultsCsv(*ccx_core.results).result_csv_job_dat_section(job_id, result_set, time, overwrite, save_filepath);
}

bool CalculiXResultsInterface::create_result(int job_id)
{
  return ccx_core.results->create_result(job_id);
}

bool CalculiXResultsInterface::delete_result(int job_id)
{
  return ccx_core.results->delete_result(job_id);
}

bool CalculiXResultsInterface::load_result(int job_id)
{
  return ccx_core.results->load_result(job_id);
}

int CalculiXResultsInterface::convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids)
{
  return ccx_core.results->convert_result(job_id, options, block_ids, nodeset_ids, sideset_ids);
}

bool CalculiXResultsInterface::project_result(int job_id, int step, int totalincrement, double scale)
{
  return ccx_core.results->project_result(job_id, step, totalincrement, scale);
}

bool CalculiXResultsInterface::project_topology(int topology_id, int iteration_id, double threshold, bool preview)
{
  if (!ccx_core.results)
  {
    PRINT_ERROR("Results core is not initialized.\n");
    return false;
  }

  return ccx_core.results->project_topology(topology_id, iteration_id, threshold, preview);
}

double CalculiXResultsInterface::compute_von_mises_stress(std::vector<double> vec)
{
  return CoreResultsMath::compute_von_mises_stress(vec);
}

double CalculiXResultsInterface::compute_von_mises_strain(std::vector<double> vec)
{
  return CoreResultsMath::compute_von_mises_strain(vec);
}

std::vector<double> CalculiXResultsInterface::compute_principal_stresses(std::vector<double> vec)
{
  return CoreResultsMath::compute_principal_stresses(vec);
}

std::vector<double> CalculiXResultsInterface::compute_principal_strains(std::vector<double> vec)
{
  return CoreResultsMath::compute_principal_strains(vec);
}
