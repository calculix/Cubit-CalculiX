#ifndef CORESTEPS_HPP
#define CORESTEPS_HPP

class CalculiXCoreInterface;

class CoreSteps
{

public:
  CoreSteps();
  ~CoreSteps();

  // make 1 data storage for each step parameter
  // NAME
  // PARAMETER STEP
  // STATIC
  // FREQUENCY
  // BUCKLE
  // HEAT TRANSFER
  // COUPLED TEMPERATURE-DISPLACEMENT
  // UNCOUPLED TEMPERATURE-DISPLACEMENT
  // FORCES
  // BCS

  // STEP TYPES
  // 1: NO ANALYSIS
  // 2: STATIC
  // 3: FREQUENCY
  // 4: BUCKLE
  // 5: HEAT TRANSFER
  // 6: COUPLED TEMPERATURE-DISPLACEMENT
  // 7: UNCOUPLED TEMPERATURE-DISPLACEMENT

  std::vector<std::vector<int>> steps_data; // used to store the connection between a step id and step parameter id
  // steps_data[0][0] step_id
  // steps_data[0][1] name_id             option 0
  // steps_data[0][2] parameter_id        option 1
  // steps_data[0][3] step_type           option 2
  // steps_data[0][4] step_type_id        option 3
  // steps_data[0][5] loads_id            option 4
  // steps_data[0][6] bcs_id              option 5
  // steps_data[0][7] historyoutputs_id   option 6
  // steps_data[0][8] fieldoutputs_id     option 7

  std::vector<std::vector<std::string>> name_data;
  // name_data[0][0] name_id
  // name_data[0][1] name

  std::vector<std::vector<std::string>> parameter_data;
  // parameter_data[0][0] parameter_id
  // parameter_data[0][1] PERTURBATION
  // parameter_data[0][2] NLGEOM {NLGEOM|NLGEOM=NO}
  // parameter_data[0][3] INC
  // parameter_data[0][4] INCF
  // parameter_data[0][5] THERMAL NETWORK
  // parameter_data[0][6] AMPLITUDE {AMPLITUDE=RAMP|AMPLITUDE=STEP}
  // parameter_data[0][7] SHOCK SMOOTHING

  std::vector<std::vector<std::string>> static_data;
  // static_data[0][0] static_id
  // static_data[0][1] solver
  // static_data[0][2] direct
  // static_data[0][3] explicit
  // static_data[0][4] time reset
  // static_data[0][5] total time at start
  // static_data[0][6] initial time increment
  // static_data[0][7] time period of the step
  // static_data[0][8] minimum time increment allowed
  // static_data[0][9] maximum time increment allowed
  // static_data[0][10] initial time increment CFD

  std::vector<std::vector<std::string>> frequency_data;
  // frequency_data[0][0] frequency_id
  // frequency_data[0][1] solver
  // frequency_data[0][2] storage
  // frequency_data[0][3] global
  // frequency_data[0][4] cycmpc
  // frequency_data[0][5] number of eigenfrequencies desired
  // frequency_data[0][6] lower value
  // frequency_data[0][7] upper value

  std::vector<std::vector<std::string>> buckle_data;
  // buckle_data[0][0] buckle_id
  // buckle_data[0][1] solver
  // buckle_data[0][2] number of buckling factors
  // buckle_data[0][3] accuracy
  // buckle_data[0][4] lanczos vectors
  // buckle_data[0][5] maximum iterations

  std::vector<std::vector<std::string>> heattransfer_data;
  // heattransfer_data[0][0] heattransfer_id
  // heattransfer_data[0][1] solver
  // heattransfer_data[0][2] direct
  // heattransfer_data[0][3] steady state
  // heattransfer_data[0][4] frequency
  // heattransfer_data[0][5] modal dynamic
  // heattransfer_data[0][6] storage
  // heattransfer_data[0][7] deltmx
  // heattransfer_data[0][8] time reset
  // heattransfer_data[0][9] total time at start
  // heattransfer_data[0][10] initial time increment
  // heattransfer_data[0][11] time period of the step
  // heattransfer_data[0][12] minimum time increment allowed
  // heattransfer_data[0][13] maximum time increment allowed
  // heattransfer_data[0][14] number of eigenfrequencies desired
  // heattransfer_data[0][15] lower value
  // heattransfer_data[0][16] upper value

  //COUPLED TEMPERATURE-DISPLACEMENT
  std::vector<std::vector<std::string>> coupledtd_data;
  // coupledtd_data[0][0] coupledtd_id
  // coupledtd_data[0][1] solver
  // coupledtd_data[0][2] direct
  // coupledtd_data[0][3] alpha
  // coupledtd_data[0][4] steady state
  // coupledtd_data[0][5] deltmx
  // coupledtd_data[0][6] time reset
  // coupledtd_data[0][7] total time at start
  // coupledtd_data[0][8] compressible
  // coupledtd_data[0][9] initial time increment
  // coupledtd_data[0][10] time period of the step
  // coupledtd_data[0][11] minimum time increment allowed
  // coupledtd_data[0][12] maximum time increment allowed
  // coupledtd_data[0][13] initial time increment CFD

  //UNCOUPLED TEMPERATURE-DISPLACEMENT
  std::vector<std::vector<std::string>> uncoupledtd_data;
  // uncoupledtd_data[0][0] uncoupledtd_id
  // uncoupledtd_data[0][1] solver
  // uncoupledtd_data[0][2] direct
  // uncoupledtd_data[0][3] alpha
  // uncoupledtd_data[0][4] steady state
  // uncoupledtd_data[0][5] deltmx
  // uncoupledtd_data[0][6] explicit
  // uncoupledtd_data[0][7] time reset
  // uncoupledtd_data[0][8] total time at start
  // uncoupledtd_data[0][9] initial time increment
  // uncoupledtd_data[0][10] time period of the step
  // uncoupledtd_data[0][11] minimum time increment allowed
  // uncoupledtd_data[0][12] maximum time increment allowed

  std::vector<std::vector<int>> loads_data;
  // forces_data[0][0] loads_id
  // forces_data[0][1] load_type  // 1: force | 2: pressure | 3: heatflux | 4: gravity | 5: centrifugal
  // forces_data[0][2] load_id

  std::vector<std::vector<int>> bcs_data;
  // bcs_data[0][0] bcs_id
  // bcs_data[0][1] bc_type       // 1: displacement | 2: temperature
  // bcs_data[0][2] bc_id

  std::vector<std::vector<int>> historyoutputs_data;
  // historyoutputs_data[0][0] historyoutputs_id
  // historyoutputs_data[0][1] historyoutput_id

  std::vector<std::vector<int>> fieldoutputs_data;
  // fieldoutputs_data[0][0] fieldoutputs_id
  // fieldoutputs_data[0][1] fieldoutput_id

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the amplitude
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_step(std::vector<std::string> options); // adds new step
  bool modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a step
  bool add_loads(int step_id, int load_type, std::vector<int> load_ids); // adds loads to loads_data
  bool add_bcs(int step_id, int bc_type, std::vector<int> bc_ids); // adds bcs to bcs_data
  bool add_historyoutputs(int step_id, std::vector<int> historyoutput_ids); // adds historyoutputs to historyoutputs_data
  bool add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids); // adds fieldoutputs to fieldoutputs_data
  bool remove_loads(int step_id, int load_type, std::vector<int> load_ids); // removes loads from loads_data
  bool remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids); // removes bcs from bcs_data
  bool remove_historyoutputs(int step_id, std::vector<int> historyoutput_ids); // removes historyoutputs from historyoutputs_data
  bool remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids); // removes fieldoutputs from fieldoutputs_data
  bool delete_step(int step_id); // deletes step from steps_data
  bool add_step(int step_id, int name_id, int parameter_id, int step_type, int step_type_id, int loads_id, int bcs_id, int historyoutputs_id, int fieldoutputs_id); // adds new step to steps_data
  bool add_name(std::string name_id, std::string name); // adds new name to name_data
  bool add_parameter(std::string parameter_id); // adds new parameter to parameter_data
  bool add_static(std::string static_id); // adds new static to static_data
  bool add_frequency(std::string frequency_id); // adds new frequency to frequency_data
  bool add_buckle(std::string buckle_id); // adds new buckle to buckle_data
  bool add_heattransfer(std::string heattransfer_id); // adds new heattransfer to heattransfer_data
  bool add_coupledtd(std::string coupledtd_id); // adds new coupledtd to coupledtd_data
  bool add_uncoupledtd(std::string uncoupledtd_id); // adds new uncoupledtd to uncoupledtd_data
  bool add_load(int loads_id, int load_type, int load_id); // adds new load to loads_data
  bool add_bc(int bcs_id, int bc_type, int bc_id); // adds new bc to bcs_data
  bool add_historyoutput(int historyoutputs_id, int historyoutput_id); // adds new historyoutput to historyoutputs_data
  bool add_fieldoutput(int fieldoutputs_id, int fieldoutput_id); // adds new fieldoutput to fieldoutputs_data
  int  get_steps_data_id_from_step_id(int step_id); // searches for the step_id in the steps_data and returns the indices or -1 if it fails
  int  get_name_data_id_from_name_id(int name_id); // searches for the name_id in the name_data and returns the indices or -1 if it fails
  int  get_parameter_data_id_from_parameter_id(int parameter_id); // searches for the parameter_id in the parameter_data and returns the indices or -1 if it fails
  int  get_static_data_id_from_static_id(int static_id); // searches for the static_id in the static_data and returns the indices or -1 if it fails
  int  get_frequency_data_id_from_frequency_id(int frequency_id); // searches for the frequency_id in the frequency_data and returns the indices or -1 if it fails
  int  get_buckle_data_id_from_buckle_id(int buckle_id); // searches for the buckle_id in the buckle_data and returns the indices or -1 if it fails
  int  get_heattransfer_data_id_from_heattransfer_id(int heattransfer_id); // searches for the heattransfer_id in the heattransfer_data and returns the indices or -1 if it fails
  int  get_coupledtd_data_id_from_coupledtd_id(int coupledtd_id); // searches for the coupledtd_id in the coupledtd_data and returns the indices or -1 if it fails
  int  get_uncoupledtd_data_id_from_uncoupledtd_id(int uncoupledtd_id); // searches for the uncoupledtd_id in the uncoupledtd_data and returns the indices or -1 if it fails
  int  get_load_data_id(int loads_id, int load_type,int load_id); // searches for the load_id in the load_data and returns the indices or -1 if it fails
  int  get_bc_data_id(int bcs_id, int bc_type, int bc_id); // searches for the bc_id in the bc_data and returns the indices or -1 if it fails
  int  get_historyoutput_data_id(int historyoutputs_id, int historyoutput_id); // searches for the historyoutput_id in the historyoutput_data and returns the indices or -1 if it fails
  int  get_fieldoutput_data_id(int fieldoutputs_id, int fieldoutput_id); // searches for the historyoutput_id in the historyoutput_data and returns the indices or -1 if it fails
  std::vector<int> get_load_data_ids_from_loads_id(int loads_id); // searches for the loads_id in the loads_data and returns the indices or -1 if it fails
  std::vector<int> get_bc_data_ids_from_bcs_id(int bcs_id); // searches for the bcs_id in the bcs_data and returns the indices or -1 if it fails
  std::vector<int> get_historyoutput_data_ids_from_historyoutputs_id(int historyoutputs_id); // searches for the historyoutputs_id in the historyoutputs_data and returns the indices or -1 if it fails
  std::vector<int> get_fieldoutput_data_ids_from_fieldoutputs_id(int fieldoutputs_id); // searches for the historyoutputs_id in the historyoutputs_data and returns the indices or -1 if it fails
  std::string get_step_export(int step_id); // get steps data export for given id
  std::string print_data(); // prints out the data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORESTEPS_HPP