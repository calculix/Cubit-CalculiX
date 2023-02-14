#ifndef COREAMPLITUDES_HPP
#define COREAMPLITUDES_HPP

class CalculiXCoreInterface;

class CoreAmplitudes
{

public:
  CoreAmplitudes();
  ~CoreAmplitudes();

  // make 1 data storage for each amplitude parameter
  // NAME
  // SHIFTX
  // SHIFTY
  // amplitudevalues

  std::vector<std::vector<int>> amplitudes_data; // used to store the connection between a amplitude id and amplitude parameter id
  // amplitudes_data[0][0] amplitude_id
  // amplitudes_data[0][1] name_id           option 0
  // amplitudes_data[0][2] shiftx_id         option 1
  // amplitudes_data[0][3] shifty_id         option 2
  // amplitudes_data[0][4] time_type   -1 for standard 1 for total time option 3
  // amplitudes_data[0][5] amplitudevalues_id   option2

  std::vector<std::vector<std::string>> name_amplitude_data;
  // name_amplitude_data[0][0] name_amplitude_id
  // name_amplitude_data[0][1] name

  std::vector<std::vector<std::string>> shiftx_amplitude_data;
  // shiftx_amplitude_data[0][0] shiftx_amplitude_id
  // shiftx_amplitude_data[0][1] shiftx_value

  std::vector<std::vector<std::string>> shifty_amplitude_data;
  // shifty_amplitude_data[0][0] shifty_amplitude_id
  // shifty_amplitude_data[0][1] shifty_value

  std::vector<std::vector<std::string>> amplitudevalues_amplitude_data;
  // amplitudevalues_amplitude_data[0][0] amplitudevalues_amplitude_id
  // amplitudevalues_amplitude_data[0][1] amplitudevalues_value
  // amplitudevalues_amplitude_data[0][2] amplitudevalues_value

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the amplitude
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds new amplitude
  bool modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a amplitude
  bool delete_amplitude(int amplitude_id); // deletes amplitude from amplitudes_data
  bool add_amplitude(int amplitude_id, int name_id, int shiftx, int shifty, int time_type, int amplitudevalues_id); // adds new amplitude to amplitudes_data
  bool add_name_amplitude(std::string name_amplitude_id, std::string name); // adds new name to name_amplitude_data
  bool add_shiftx_amplitude(std::string shiftx_amplitude_id, std::string shiftx_value); // adds new shiftxvalue to shiftx_amplitude_data
  bool add_shifty_amplitude(std::string shifty_amplitude_id, std::string shifty_value); // adds new shiftyvalue to shifty_amplitude_data
  bool add_amplitudevalues_amplitude(std::string amplitudevalues_amplitude_id, std::string amplitudevalues_value1, std::string amplitudevalues_value2); // adds new amplitudevalues to amplitudevalues_amplitude_data
  int  get_amplitudes_data_id_from_amplitude_id(int amplitude_id); // searches for the amplitude_id in the amplitudes_data and returns the indices or -1 if it fails
  int  get_name_amplitude_data_id_from_name_amplitude_id(int name_amplitude_id); // searches for the name_amplitude_id in the name_amplitude_data and returns the indices or -1 if it fails
  int  get_shiftx_amplitude_data_id_from_shiftx_amplitude_id(int shiftx_amplitude_id); // searches for the shiftx_amplitude_id in the shiftx_amplitude_data and returns the indices or -1 if it fails
  int  get_shifty_amplitude_data_id_from_shifty_amplitude_id(int shifty_amplitude_id); // searches for the shifty_amplitude_id in the shifty_amplitude_data and returns the indices or -1 if it fails
  std::vector<int> get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id(int amplitudevalues_amplitude_id); // searches for the amplitudevalues_amplitude_id in the amplitudevalues_amplitude_data and returns the indices or -1 if it fails
  std::string get_amplitude_export(); // get CalculiX amplitude exports
  std::string print_data(); // prints out the blocks_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREAMPLITUDES_HPP
