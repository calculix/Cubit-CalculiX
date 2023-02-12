#ifndef CORECONTACTPAIRS_HPP
#define CORECONTACTPAIRS_HPP

class CalculiXCoreInterface;

class CoreContactPairs
{

public:
  CoreContactPairs();
  ~CoreContactPairs();

  // make 1 data storage for each contact pair type
  // adjust

  std::vector<std::vector<int>> contactpairs_data; // used to store the connection between a constraint id and constraint type id
  // contactpairs_data[0][0] contactpair_id
  // contactpairs_data[0][1] surfaceinteractions_id
  // contactpairs_data[0][2] contactpair_type
  // contactpairs_data[0][3] master_id  must be a sideset id
  // contactpairs_data[0][4] slave_id   must be a sideset id
  // contactpairs_data[0][5] adjust_id
  
  //contactpairs_type:  1 - NODE TO SURFACE
  //                    2 - SURFACE TO SURFACE
  //                    3 - MORTAR
  //                    4 - LINMORTAR
  //                    5 - PGLINMORTAR
  //                    6 - MASSLESS

  std::vector<std::vector<std::string>> adjust_contactpair_data;
  // adjust_contactpair_data[0][0] adjust_contactpair_id
  // adjust_contactpair_data[0][1] adjust_value 

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the constraints
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_contactpair(std::string contactpair_type, std::vector<std::string> options); // adds new contactpair
  bool modify_contactpair(std::string contactpair_type, int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a contactpair
  bool delete_contactpair(int contactpair_id); // deletes contactpair from contactpairs_data
  bool add_contactpair(int constraint_id, int constraint_type, int constraint_type_id); // adds new constraint to constraints_data
  bool add_adjust_contactpair(std::string adjust_contactpair_id, std::string adjust_value); // adds new adjustvalue to adjust_contactpair_data
  int  get_contactpairs_data_id_from_contactpair_id(int contactpair_id); // searches for the contactpair_id in the contactpairs_data and returns the indices or -1 if it fails
  int  get_adjust_contactpair_data_id_from_adjust_contactpair_id(int adjust_contactpair_id); // searches for the adjust_contactpair_id in the adjust_contactpair_data and returns the indices or -1 if it fails
  std::string get_contactpair_export(); // get CalculiX constraint exports
  std::string print_data(); // prints out the blocks_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORECONTACTPAIRS_HPP
