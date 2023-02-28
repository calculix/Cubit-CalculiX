#ifndef COREFIELDOUTPUTS_HPP
#define COREFIELDOUTPUTS_HPP

class CalculiXCoreInterface;

class CoreFieldOutputs
{
public:
  CoreFieldOutputs();
  ~CoreFieldOutputs();

  // make 1 data storage for each history output parameter
  // NAME
  // NODE PRINT
  // EL PRINT
  // CONTACT PRINT

  std::vector<std::vector<int>> outputs_data; // used to store the connection between a output id and output parameter id
  // outputs_data[0][0] output_id
  // outputs_data[0][1] name_id           option 0
  // outputs_data[0][2] output_type 1 node print, 2 el print, 3 contact print       option 1
  // outputs_data[0][3] output_type_id         

  std::vector<std::vector<std::string>> name_data;
  // name_data[0][0] name_id
  // name_data[0][1] name

  std::vector<std::string> node_keys;
  std::vector<std::vector<std::string>> node_data;
  // node_data[0][0] node_id
  // node_data[0][1] nodeset_id NSET      option[0]...
  // node_data[0][2] FREQUENCY
  // node_data[0][3] FREQUENCYF
  // node_data[0][4] TOTALS
  // node_data[0][5] GLOBAL
  // node_data[0][6] OUTPUT
  // node_data[0][7] OUTPUT ALL
  // node_data[0][8] TIME POINTS
  // node_data[0][9] LAST ITERATIONS
  // node_data[0][10] CONTACT ELEMENTS
  // node_data[0][11] KEY: CP
  // node_data[0][12] KEY: DEPF
  // node_data[0][13] KEY: DEPT
  // node_data[0][14] KEY: DTF
  // node_data[0][15] KEY: HCRI
  // node_data[0][16] KEY: KEQ
  // node_data[0][17] KEY: MACH
  // node_data[0][18] KEY: MAXU
  // node_data[0][19] KEY: MF
  // node_data[0][20] KEY: NT
  // node_data[0][21] KEY: PNT
  // node_data[0][22] KEY: POT
  // node_data[0][23] KEY: PRF
  // node_data[0][24] KEY: PS
  // node_data[0][25] KEY: PSF
  // node_data[0][26] KEY: PT
  // node_data[0][27] KEY: PTF
  // node_data[0][28] KEY: PU
  // node_data[0][29] KEY: RF
  // node_data[0][30] KEY: RFL
  // node_data[0][31] KEY: SEN
  // node_data[0][32] KEY: TS
  // node_data[0][33] KEY: TSF
  // node_data[0][34] KEY: TT
  // node_data[0][35] KEY: TTF
  // node_data[0][36] KEY: TURB
  // node_data[0][37] KEY: U
  // node_data[0][38] KEY: V
  // node_data[0][39] KEY: VF
  
  std::vector<std::string> element_keys;
  std::vector<std::vector<std::string>> element_data;
  // element_data[0][0] element_id            
  // element_data[0][1] nodeset_id NSET   option[0]...
  // element_data[0][2] FREQUENCY
  // element_data[0][3] FREQUENCYF
  // element_data[0][4] GLOBAL
  // element_data[0][5] OUTPUT
  // element_data[0][6] OUTPUT ALL
  // element_data[0][7] SECTION FORCES
  // element_data[0][8] TIME POINTS
  // element_data[0][9] LAST ITERATIONS
  // element_data[0][10] CONTACT ELEMENTS
  // element_data[0][11] KEY: CEEQ
  // element_data[0][12] KEY: E
  // element_data[0][13] KEY: ECD
  // element_data[0][14] KEY: EMFB
  // element_data[0][15] KEY: EMFE
  // element_data[0][16] KEY: ENER
  // element_data[0][17] KEY: ERR
  // element_data[0][18] KEY: HER
  // element_data[0][19] KEY: HFL
  // element_data[0][20] KEY: HFLF
  // element_data[0][21] KEY: MAXE
  // element_data[0][22] KEY: MAXS
  // element_data[0][23] KEY: ME
  // element_data[0][24] KEY: PEEQ
  // element_data[0][25] KEY: PHS
  // element_data[0][26] KEY: S
  // element_data[0][27] KEY: SF
  // element_data[0][28] KEY: SMID
  // element_data[0][29] KEY: SNEG
  // element_data[0][30] KEY: SPOS
  // element_data[0][31] KEY: SVF
  // element_data[0][32] KEY: SDV
  // element_data[0][33] KEY: THE
  // element_data[0][34] KEY: ZZS
  
  std::vector<std::string> contact_keys;
  std::vector<std::vector<std::string>> contact_data;
  // contact_data[0][0] contact_id              
  // contact_data[0][2] FREQUENCY   option[0]...
  // contact_data[0][3] TIME POINTS
  // contact_data[0][4] LAST ITERATIONS
  // contact_data[0][5] CONTACT ELEMENTS
  // contact_data[0][6] KEY: CDIS
  // contact_data[0][7] KEY: CSTR
  // contact_data[0][8] KEY: CELS
  // contact_data[0][9] KEY: PCON
    
  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the amplitude
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_output(std::vector<std::string> options); // adds new output
  bool modify_output(int output_id,int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a output
  bool delete_output(int output_id); // deletes output from outputs_data
  bool add_output(int output_id, int name_id, int output_type, int output_type_id); // adds new output to outputs_data
  bool add_name(std::string name_id, std::string name); // adds new name to name_data
  bool add_node(std::string node_id); // adds new node to node_data with empty data
  bool add_element(std::string element_id); // adds new element to element_data with empty data
  bool add_contact(std::string contact_id); // adds new contact to contact_data with empty data
  int  get_outputs_data_id_from_output_id(int output_id); // searches for the output_id in the outputs_data and returns the indices or -1 if it fails
  int  get_name_data_id_from_name_id(int name_id); // searches for the name_id in the name_data and returns the indices or -1 if it fails
  int  get_node_data_id_from_node_id(int node_id); // searches for the node_id in the node_data and returns the indices or -1 if it fails
  int  get_element_data_id_from_element_id(int element_id); // searches for the element_id in the element_data and returns the indices or -1 if it fails
  int  get_contact_data_id_from_contact_id(int contact_id); // searches for the contact_id in the contact_data and returns the indices or -1 if it fails
  std::string get_output_export(std::vector<int> output_ids); // get CalculiX output exports
  std::string print_data(); // prints out the data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREFIELDOUTPUTS_HPP