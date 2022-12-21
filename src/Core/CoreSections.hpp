#ifndef CORESECTIONS_HPP
#define CORESECTIONS_HPP

class CoreSections
{

public:
  CoreSections();
  ~CoreSections();

  // make 1 data storage for each section type
  // solid
  // shell
  // beam

  std::vector<std::vector<int>> sections_data; // used to store the connection between a section id and section type id
  // sections_data[0][0] section_id
  // sections_data[0][1] section_type
  // sections_data[0][2] section_type_id

  std::vector<std::vector<int>> solid_section_data; // type 1
  std::vector<std::vector<int>> shell_section_data; // type 2
  std::vector<std::vector<int>> beam_section_data;  // type 3
  std::vector<std::vector<int>> membrane_section_data;// type 4

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the sections
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool add_section(int section_id, int section_type, int section_type_id); // adds new section to sections_data
  bool delete_section(int section_id); // deletes section from sections_data
  bool modify_section_id(int section_id, int new_section_id); // modifies the section_id from given section
  int  get_sections_data_id_from_section_id(int section_id); // searches for the section_id in the sections_data and returns the indices or -1 if it fails
  std::string print_data(); // prints out the blocks_data
};

#endif // CORESECTIONS_HPP
