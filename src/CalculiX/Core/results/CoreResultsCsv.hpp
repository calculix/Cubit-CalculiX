#ifndef CORERESULTSCSV_HPP
#define CORERESULTSCSV_HPP

#include "frd/CoreResultsFrdQuery.hpp"
#include "dat/CoreResultsDatQuery.hpp"

#include <string>

class CoreResults;

class CoreResultsCsv
{
public:
  explicit CoreResultsCsv(CoreResults& results);

  bool result_csv_job_frd(
    int job_id, std::string block_type, std::string block_component, std::string increment,
    int node_id, int block_id, int nodeset_id, int sideset_id,
    bool overwrite, std::string save_filepath);

  bool result_csv_job_dat(
    int job_id, std::string block_type, std::string block_set, std::string block_component,
    std::string time, int node_id, int element_id, bool overwrite, std::string save_filepath);

  bool result_csv_job_dat_section(
    int job_id, std::string result_set, std::string time, bool overwrite, std::string save_filepath);

private:
  CoreResultsFrdQuery frd_query;
  CoreResultsDatQuery dat_query;
};

#endif
