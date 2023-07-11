#ifndef CORECUSTOMLINES_HPP
#define CORECUSTOMLINES_HPP

class CalculiXCoreInterface;

class CoreCustomLines
{

public:
  CoreCustomLines();
  ~CoreCustomLines();

  std::vector<std::vector<std::string>> customlines_data;
  // customlines_data[0][0] customline_id
  // customlines_data[0][1] name
  // customlines_data[0][2] position
  // customlines_data[0][3] insert keyword
  // customlines_data[0][4] insert keyword id
  // customlines_data[0][5] customline

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the customlines
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_customline(std::vector<std::string> options); // adds new customline
  bool modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a customline
  bool delete_customline(int customline_id); // deletes customline from customlines_data
  bool add_customline(int customline_id, std::string name, std::string position, std::string keyword,std::string keyword_id, std::string customline); // adds new customline to customlines_data
  int  get_customlines_data_id_from_customline_id(int customline_id); // searches for the customline_id in the customlines_data and returns the indices or -1 if it fails
  std::vector<std::string> get_customline_data(std::string position,std::string keyword,int keyword_id); // searches and returns output for asked insert point
  std::string print_data(); // prints out the customlines_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORECUSTOMLINES_HPP