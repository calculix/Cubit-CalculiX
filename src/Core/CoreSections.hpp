#ifndef CORESECTIONS_HPP
#define CORESECTIONS_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreSections
{

public:
  CoreSections();
  ~CoreSections();

  // make 1 data storage for each section type
  // solid
  // shell
  // beam
  // membrane

  std::vector<std::vector<int>> sections_data; // used to store the connection between a section id and section type id
  // sections_data[0][0] section_id
  // sections_data[0][1] section_type
  // sections_data[0][2] section_type_id

  std::vector<std::vector<std::string>> solid_section_data; // type 1
  // solid_section_data[0][0] solid_section_id
  // solid_section_data[0][1] block_id
  // solid_section_data[0][2] material
  // solid_section_data[0][3] option: orientation id
  // solid_section_data[0][4] option: thickness
  
  std::vector<std::vector<std::string>> shell_section_data; // type 2
  // shell_section_data[0][0] shell_section_id
  // shell_section_data[0][1] block_id
  // shell_section_data[0][2] material_id
  // shell_section_data[0][3] option: orientation
  // shell_section_data[0][4] option: thickness
  // shell_section_data[0][5] option: offset

  std::vector<std::vector<std::string>> beam_section_data;  // type 3
  // beam_section_data[0][0] shell_section_id
  // beam_section_data[0][1] block_id
  // beam_section_data[0][2] material_id
  // beam_section_data[0][3] beam_type
  // beam_section_data[0][4] Parameter 1
  // beam_section_data[0][5] Parameter 2
  // beam_section_data[0][6] Parameter 3
  // beam_section_data[0][7] Parameter 4
  // beam_section_data[0][8] Parameter 5
  // beam_section_data[0][9] Parameter 6
  // beam_section_data[0][10] x
  // beam_section_data[0][11] y
  // beam_section_data[0][12] z
  // beam_section_data[0][13] option: orientation id
  // beam_section_data[0][14] option: offset1
  // beam_section_data[0][15] option: offset2

  std::vector<std::vector<std::string>> membrane_section_data;// type 4
  // membrane_section_data[0][0] shell_section_id
  // membrane_section_data[0][1] block_id
  // membrane_section_data[0][2] material_id
  // membrane_section_data[0][3] option: orientation id
  // membrane_section_data[0][4] option: thickness
  // membrane_section_data[0][5] option: offset

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the sections
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options); // adds new section
  bool modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a section
  bool delete_section(int section_id); // deletes section from sections_data
  bool add_section(int section_id, int section_type, int section_type_id); // adds new section to sections_data
  bool add_solid_section(std::string solid_section_id, std::string block_id, std::string material_id,std::string orientation,std::string thickness); // adds new solid section to solid_sections_data
  bool add_shell_section(std::string shell_section_id, std::string block_id, std::string material_id,std::string orientation,std::string thickness,std::string offset); // adds new shell section to shell_sections_data
  bool add_beam_section(std::string beam_section_id, std::string block_id, std::string material_id,std::string beam_type,std::string parameter1,std::string parameter2,std::string parameter3,std::string parameter4,std::string parameter5,std::string parameter6,std::string x,std::string y,std::string z,std::string orientation,std::string offset1,std::string offset2); // adds new beam section to beam_sections_data
  bool add_membrane_section(std::string membrane_section_id, std::string block_id, std::string material_id,std::string orientation,std::string thickness,std::string offset); // adds new membrane section to membrane_sections_data
  bool modify_section_id(int section_id, int new_section_id); // modifies the section_id from given section
  int  get_sections_data_id_from_section_id(int section_id); // searches for the section_id in the sections_data and returns the indices or -1 if it fails
  int  get_solid_section_data_id_from_solid_section_id(int solid_section_id); // searches for the solid_section_id in the solid_section_data and returns the indices or -1 if it fails
  int  get_shell_section_data_id_from_shell_section_id(int shell_section_id); // searches for the shell_section_id in the shell_section_data and returns the indices or -1 if it fails
  int  get_beam_section_data_id_from_beam_section_id(int beam_section_id); // searches for the beam_section_id in the beam_section_data and returns the indices or -1 if it fails
  int  get_membrane_section_data_id_from_membrane_section_id(int membrane_section_id); // searches for the membrane_section_id in the membrane_section_data and returns the indices or -1 if it fails
  std::string get_section_export(); // get CalculiX section exports
  std::string print_data(); // prints out the blocks_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORESECTIONS_HPP
