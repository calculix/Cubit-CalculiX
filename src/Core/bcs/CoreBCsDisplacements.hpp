#ifndef COREBCSDISPLACEMENTS_HPP
#define COREBCSDISPLACEMENTS_HPP

#include <vector>
#include <string>

class CoreBCsDisplacements
{
public:
  CoreBCsDisplacements();
  ~CoreBCsDisplacements();

  // bc_id, op_mode, amplitude_id, time_delay_id, nodeset_id, displacement_id, name_id
  std::vector<std::vector<int>> bcs_data;
  // displacement_id, dof_1, dof_2, dof_3, dof_4, dof_5, dof_6
  std::vector<std::vector<double>> displacement_data;
  // displacement_id, mode_1..6: 0 free, 1 prescribed, 2 FIXED
  std::vector<std::vector<int>> mode_data;
  // time_delay_id, time_delay_value
  std::vector<std::vector<std::string>> time_delay_data;
  // name_id, name
  std::vector<std::vector<std::string>> name_data;

  bool is_initialized = false;

  bool init();
  bool update();
  bool reset();
  bool check_initialized();
  bool create_bc(std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3);
  bool add_bc(int bc_id, int op_mode, int amplitude_id, int time_delay_id, int nodeset_id, int displacement_id, int name_id);
  bool modify_bc(int bc_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3, std::vector<int> options_marker);
  bool delete_bc(int bc_id);
  bool add_time_delay(std::string time_delay_id, std::string time_delay_value);
  bool add_displacement(double displacement_id, std::vector<double> values);
  bool add_mode(int displacement_id, std::vector<int> modes);
  bool add_name(std::string name_id, std::string name);
  int get_bcs_data_id_from_bc_id(int bc_id);
  int get_time_delay_data_id_from_time_delay_id(int time_delay_id);
  int get_displacement_data_id_from_displacement_id(int displacement_id);
  int get_mode_data_id_from_displacement_id(int displacement_id);
  int get_name_data_id_from_name_id(int name_id);
  std::string get_bc_export(int bc_id);
  std::string get_hbc_export(int bc_id);
  std::string get_initial_displacement_export(int bc_id);
  std::string get_bc_parameter_export(int bc_id);
  std::string get_bc_parameter_fixed_export(int bc_id);
  bool check_fixed(int bc_id);
  bool check_dof_fixed(int bc_id, int dof);
  std::string print_data();
};

#endif // COREBCSDISPLACEMENTS_HPP