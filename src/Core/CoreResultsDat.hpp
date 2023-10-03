#ifndef CORERESULTSDAT_HPP
#define CORERESULTSDAT_HPP

class CalculiXCoreInterface;

class CoreResultsDat
{

public:
  CoreResultsDat();
  ~CoreResultsDat();

  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;

  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool read(); // reads the dat file

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSDAT_HPP