#ifndef CALCULIXINTERACTIONSINTERFACE_HPP
#define CALCULIXINTERACTIONSINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXInteractionsInterface
{
public:
  std::string get_surfaceinteraction_name(int surfaceinteraction_id); // gets the surfaceinteraction name
  std::vector<std::string> get_contactpair_master_slave(int contactpair_id); // gets the contactpair master and slave name
  std::vector<int> get_equation_ids(); // get all equation ids
  std::vector<int> get_equationgroup_ids(); // get all equation ids
  std::string get_constraint_export_data(); // gets the export data from constraints core
  std::string get_surfaceinteraction_export_data(); // gets the export data from surfaceinteractions core
  std::string get_contactpair_export_data(); // gets the export data from contactpairs core
  bool create_constraint(std::string constraint_type, std::vector<std::string> options, std::vector<std::vector<double>> options2); // adds a new constraint
  bool modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<double>> options2); // modify a constraint
  bool delete_constraint(int constraint_id); // delete constraint
  bool create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance); // create constraint tie from cubit contact pairs
  bool create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3); // create constraint equation from coincident nodes
  bool create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new surfaceinteraction
  bool modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modifies a surfaceinteraction
  bool delete_surfaceinteraction(int surfaceinteraction_id); // delete surface interaction
  bool create_contactpair(std::vector<std::string> options); // adds a new contactpair
  bool modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a contactpair
  bool delete_contactpair(int contactpair_id); // delete contact pair
  bool create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset); // create contact pairs from cubit contact pairs with given surface interaction
  std::vector<std::vector<std::string>> get_constraints_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_surfaceinteractions_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_contactpairs_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXINTERACTIONSINTERFACE_HPP
