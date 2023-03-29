#ifndef CORESIMULATION_HPP
#define CORESIMULATION_HPP

class CalculiXCoreInterface;

class CoreSimulation
{

public:
  CoreSimulation();
  ~CoreSimulation();

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the jobs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool solve(std::string filepath); // runs the simulation with given input file

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORESIMULATION_HPP