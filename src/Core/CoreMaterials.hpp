#ifndef COREMATERIALS_HPP
#define COREMATERIALS_HPP

class MaterialInterface;

class CoreMaterials
{

public:
  CoreMaterials();
  ~CoreMaterials();

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the materials // just a placeholder right now
  bool reset(); // delete all data and initialize afterwards // just a placeholder right now
  bool check_initialized(); // check if object is initialized
  std::vector<std::string> get_group_list(); // get a list of CalculiX Material Groups

  MaterialInterface *m_iface;
};

#endif // COREMATERIALS_HPP
