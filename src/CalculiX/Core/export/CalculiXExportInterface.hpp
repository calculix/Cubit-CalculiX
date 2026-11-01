#ifndef CALCULIXEXPORTINTERFACE_HPP
#define CALCULIXEXPORTINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXExportInterface
{
public:
  std::vector<int> get_customline_ids(); // get all equationgroup ids
  bool prepare_export(); // prepares data for export like for trajectory
  bool clean_export(); // cleans data for export like for trajectory
  bool create_customline(std::vector<std::string> options); // adds a new customline
  bool modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a customline
  bool delete_customline(int customline_id); // delete customline
  std::vector<std::string> get_customline_data(std::string position,std::string keyword,int keyword_id);
  std::vector<std::vector<std::string>> get_customlines_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXEXPORTINTERFACE_HPP
