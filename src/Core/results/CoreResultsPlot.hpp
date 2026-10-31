#ifndef CORERESULTSPLOT_HPP
#define CORERESULTSPLOT_HPP

#include "frd/CoreResultsFrdQuery.hpp"
#include "dat/CoreResultsDatQuery.hpp"

#include <QString>
#include <string>
#include <vector>

class CoreResults;

class CoreResultsPlot
{
public:
  explicit CoreResultsPlot(CoreResults& results);

  bool result_plot_job_frd(
    int job_id, int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment, bool x_time,
    int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,
    QString title, QString x_axis, QString y_axis, bool save, QString save_filepath);

  bool result_plot_job_dat(
    int job_id, int x_node_id, int x_element_id, int x_element_ip,
    std::string x_block_set, std::string x_block_type, std::string x_block_component, bool x_time,
    int y_node_id, int y_element_id, int y_element_ip,
    std::string y_block_set, std::string y_block_type, std::string y_block_component, bool y_time,
    QString title, QString x_axis, QString y_axis, bool save, QString save_filepath);

  bool result_plot_job_dat_section(
    int job_id, std::string x_section_set, std::string x_section_component, bool x_time,
    std::string y_section_set, std::string y_section_component, bool y_time,
    QString title, QString x_axis, QString y_axis, bool save, QString save_filepath);

private:
  CoreResultsFrdQuery frd_query;
  CoreResultsDatQuery dat_query;

  static bool show_plot(
    QString windowtitle, QString title, QString x_axis, QString y_axis,
    const std::vector<double>& x_data, const std::vector<double>& y_data,
    bool save, QString save_filepath);
};

#endif
