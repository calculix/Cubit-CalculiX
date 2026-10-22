#ifndef CALCULIXAPPLICATIONINTERFACE_HPP
#define CALCULIXAPPLICATIONINTERFACE_HPP

#include <string>
#include <vector>

class CoreObserver;

class CalculiXApplicationInterface
{
public:
  static CoreObserver& core_observer();
  bool cmd(std::string cmd); // executes a cubit command with appending to the history
  bool silent_cmd(std::string cmd); // executes a cubit command
  bool is_silent_cmd() const;
  std::string get_version(); // gets the component version
  bool set_use_ccx_autocleanup(bool bool_use);
  bool set_use_ccx_logfile(bool bool_use);
  bool init_pythoninterface();
  bool init_materiallibrary();
  bool init_completed(); // returns true after every init is true otherwise false
  bool gui_updated(); //returns if gui is updated, necessary because of time check in the observer
  bool set_gui_updated(bool status); //sets the bool if core the gui was updated
  bool block_core_update(); //returns the bool if the core should be updated
  bool set_block_core_update(bool status); //sets the bool if the should be updated
  bool block_gui_update(); //returns the bool if the gui should be updated
  bool set_block_gui_update(bool status); //sets the bool if the gui should be updated
  std::string print_data();
  bool print_to_log(std::string str_log);
  bool print_to_cl(std::string str_log);
  bool export_to_csv(std::string path_and_name, std::vector<std::string> header, std::vector<std::vector<double>> data,bool overwrite); // exports the data to a .csv file, with the header
  std::vector<int> parser(std::string parse_type, std::string parse_string);
  std::string to_string_scientific(double value, int precision = 6); // converts a double to string with scientific notation, with optional precision
  double string_scientific_to_double(std::string value, int precision = 6); // converts a string with scientific notation to double, with optional precision
  bool core_update(); // lets the core check for updates aka changes from the entities
  bool core_reset(); // reset the whole core to the init level
  bool core_read_cub(std::string filename); // read core data from cub
  bool core_save_cub(std::string filename); // save core data to cub
};

#endif // CALCULIXAPPLICATIONINTERFACE_HPP
