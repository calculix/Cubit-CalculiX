#ifndef CORERESULTSVTKWRITER_HPP
#define CORERESULTSVTKWRITER_HPP

class CalculiXCoreInterface;

class CoreResultsVtkWriter
{

public:
  CoreResultsVtkWriter();
  ~CoreResultsVtkWriter();
  
  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;

  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool clear(); // clear all stored result data
  bool check_initialized(); // check if object is initialized
  
  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSVTKWRITER_HPP