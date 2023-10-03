#ifndef CORERESULTSFRD_HPP
#define CORERESULTSFRD_HPP

class CalculiXCoreInterface;

class CoreResultsFrd
{

public:
  CoreResultsFrd();
  ~CoreResultsFrd();

  //nodal data
  //element data

  int job_id = -1;
  std::string filepath = "";
  bool is_initialized = false;

  bool init(int job_id); // initialize
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool read(); // reads the frd file
  std::vector<std::string> split_line(std::string frdline); // splits the line to a vector
  bool print_data(); // prints the data to the console

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTSFRD_HPP