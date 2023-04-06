#include "CoreJobs.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"


CoreJobs::CoreJobs()
{}

CoreJobs::~CoreJobs()
{}

bool CoreJobs::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

bool CoreJobs::update()
{ 
  return true;
}

bool CoreJobs::reset()
{
  jobs_data.clear();
  
  init();
  return true;
}

bool CoreJobs::check_initialized()
{
  return is_initialized;
}

bool CoreJobs::create_job(std::vector<std::string> options)
{
  int job_id;
  int job_last;
  
  if (jobs_data.size()==0)
  {
    job_id = 1;
  }
  else
  {
    job_last = jobs_data.size() - 1;
    job_id = std::stoi(jobs_data[job_last][0]) + 1;
  }

  this->add_job(job_id, options[0], options[1]);
  return true;
}

bool CoreJobs::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  int jobs_data_id = get_jobs_data_id_from_job_id(job_id);
  
  if (jobs_data_id == -1)
  {
    return false;
  } else {
    // name
    if (options_marker[0]==1)
    {
      jobs_data[jobs_data_id][1] = options[0];
    }
    // filepath
    if (options_marker[1]==1)
    {
      jobs_data[jobs_data_id][2] = options[1];
    }
    return true;
  }
}

bool CoreJobs::add_job(int job_id, std::string name, std::string filepath)
{
  std::vector<std::string> v = {std::to_string(job_id), name, filepath, "0"};      
  jobs_data.push_back(v);
  return true;
}

bool CoreJobs::delete_job(int job_id)
{
  int jobs_data_id = get_jobs_data_id_from_job_id(job_id);
  if (jobs_data_id == -1)
  {
    return false;
  } else {
    jobs_data.erase(jobs_data.begin() + jobs_data_id);
    return true;
  }
}

bool CoreJobs::run_job(int job_id)
{
  std::string filepath;
  std::string log;
  std::string command;
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);

  if (jobs_data[job_data_id][2]!="")
  {
    filepath = jobs_data[job_data_id][2];
  } else {
    filepath = jobs_data[job_data_id][1] + ".inp";
    log = "Exporting Job " + jobs_data[job_data_id][1] + " with ID " + jobs_data[job_data_id][0] + " to \n";
    log.append(filepath +  " \n");
    PRINT_INFO("%s", log.c_str());
    command = "export ccx \"" + filepath + "\"";
    CubitInterface::cmd(command.c_str());
    if (CubitInterface::was_last_cmd_undoable())
    {
      return false;
    }
  }
  start_process_linux(job_id, filepath);
  return true;
}

bool CoreJobs::start_process_linux(int job_id, std::string filepath)
{
  std::string log;
  std::string ccx_exe;
  std::string command;
  int job_data_id;
  job_data_id = get_jobs_data_id_from_job_id(job_id);

  log = "Submitting Job " + jobs_data[job_data_id][1] + " with ID " + jobs_data[job_data_id][0] + " to CCX\n";
  PRINT_INFO("%s", log.c_str());
  
  ccx_exe = "/home/user/Downloads/cgx_2.20.1";

  return true;
}

int CoreJobs::get_jobs_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < jobs_data.size(); i++)
  {
    if (jobs_data[i][0]==std::to_string(job_id))
    {
      return_int = i;
    }  
  }
  return return_int;
}

std::string CoreJobs::print_data()
{
  std::string str_return;
  str_return = "\n CoreJobs jobs_data: \n";
  str_return.append("job_id, name, filepath, status \n");

  for (size_t i = 0; i < jobs_data.size(); i++)
  {
    str_return.append(jobs_data[i][0] + " " + jobs_data[i][1] + " " + jobs_data[i][2] + " " + jobs_data[i][3] + " \n");
  }

  return str_return;
}