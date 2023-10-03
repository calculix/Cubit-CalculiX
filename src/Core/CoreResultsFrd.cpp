#include "CoreResultsFrd.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <fstream>
#include <iostream>
#include "loadUserOptions.hpp"

CoreResultsFrd::CoreResultsFrd()
{}

CoreResultsFrd::~CoreResultsFrd()
{}

bool CoreResultsFrd::init(int job_id)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    this->job_id = job_id;

    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    this->filepath = job_data[1] + ".frd";

    is_initialized = true;  
    return true;
  }
}

bool CoreResultsFrd::reset()
{
  init(-1);
  return true;
}

bool CoreResultsFrd::check_initialized()
{
  return is_initialized;
}


bool CoreResultsFrd::read()
{
  std::string log;
  log = "reading results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());
  
  std::string frdline = "";
  std::vector<std::string> frd_array;
  
  std::ifstream frd;
  frd.open(this->filepath);

  if (frd.is_open())
  {
    log = "";
    while (frd)
    {
      std::getline(frd,frdline);

      frd_array = this->split_line(frdline);

      for (size_t i = 0; i < frd_array.size(); i++)
      {
        log.append(frd_array[i] + " ");
      }

      log.append("\n");
    }
  }
  frd.close();

  PRINT_INFO("%s", log.c_str());
  print_data();
  
  return true;
}

std::vector<std::string> CoreResultsFrd::split_line(std::string frdline)
{
  std::vector<std::string> str_array;
  std::string str_temp;

  // trim whitespaces
  size_t strBegin = frdline.find_first_not_of(" \t\r\f\v\n");
  if (strBegin != std::string::npos)
  {
    size_t strEnd = frdline.find_last_not_of(" \t\r\f\v\n");
    size_t strRange = strEnd - strBegin + 1;
    frdline = frdline.substr(strBegin, strRange);
  }
  // reduce whitespaces
  size_t beginSpace = frdline.find_first_of(" \t\r\f\v\n");
  while (beginSpace != std::string::npos)
  {
    size_t endSpace = frdline.find_first_not_of(" \t\r\f\v\n", beginSpace);
    size_t range = endSpace - beginSpace;

    frdline.replace(beginSpace,range, " ");
    size_t newStart = beginSpace + 1;
    beginSpace = frdline.find_first_of(" \t\r\f\v\n",newStart);
  }
  // insert whitespaces before "-" whitespaces but not "E-"
  beginSpace = frdline.find_first_of("-");
  while (beginSpace != std::string::npos)
  {
    size_t range = 1;
    if ((frdline[beginSpace-1] != 'E') && (beginSpace !=0))
    {
      frdline.replace(beginSpace,range, " -");
    }
    size_t newStart = beginSpace + 2;
    beginSpace = frdline.find_first_of("-",newStart);
  }

  std::stringstream ss(frdline);

  // split line with delimiter " "
  while (getline(ss,str_temp,' '))
  {
    str_array.push_back(str_temp);
  }
  
  return str_array;
}

bool CoreResultsFrd::print_data()
{
  std::string log;
  log = "FRD DATA for Job ID " + std::to_string(job_id) + " \n";

  PRINT_INFO("%s", log.c_str());
  return true;
}